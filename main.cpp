// Librairies standards
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Types de données
#include <string.h>
#include <map>
#include <vector>
// Fichiers
#include <fstream>
#include <sys/stat.h>
// Date
#include <time.h>
// Maths
#include <cmath>

// Using
using namespace std;

// Config générale
int
	// Températures
	tempJour 		= 75,
	tempNuit 		= 70,
	tempCritique 	= 85,
	// Cartes
	maxCartes		= 6,
	// Vitesse ventilo
	minVentiloJour	= 100,
	minVentiloNuit	= 100
;
// Données selon période
bool nuit = false;
bool lance = false; // Si le mineur a deja été lancé
int tempLimite = tempJour;
int minVentilo = minVentiloJour;
bool modeBoost = false;

// Gestion carte
#include "carte.h"

// Déclarations
struct stat sb;
bool is_dir(string Chemin);

 // Association chemin moniteur => liste d'infos
vector<Carte> Cartes;

// Point d'entrée
int main (int argc, char *argv[]) {
	// Paramètrage des températures via arguments
	if (argc >= 2) tempJour 	= stoi(argv[1]);
	if (argc >= 3) tempNuit 	= stoi(argv[2]);
	if (argc >= 4) tempCritique = stoi(argv[3]);
	if (argc >= 5) minVentiloJour	= round((float)stoi(argv[4]) / 100 * 255);
	if (argc >= 6) minVentiloNuit	= round((float)stoi(argv[5]) / 100 * 255);

	// Récencement des cartes dediées
	cout << "Recencement des cartes ..." << endl;
	for (int numCarte = 0 ; numCarte <= 6 ; numCarte++) {
		// Collecte des infos
		int numMon = Cartes.size(); // Numéro du moniteur dispo
		string Moniteur = "/sys/class/drm/card"+ to_string(numCarte) +"/device/hwmon/hwmon"+ to_string(numMon);
		// Vérif si pas chipset
		if (is_dir(Moniteur)) {
			cout << "Carte " << to_string(numCarte) << " OK" << endl;
			Cartes.push_back(Carte(numCarte, Moniteur));
			// Initialisation
			Cartes.back().setVentilo(200);
		}
	}
	cout << to_string(Cartes.size()) << " cartes prises en charge" << endl;

	// Boucle principale
	while (1) {
        time_t theTime = time(NULL);
        struct tm *aTime = localtime(&theTime);
		int Heure = aTime->tm_hour;
		int Minutes = aTime->tm_min;
		int Secondes = aTime->tm_sec;

		// Détermination période journée + température adaptée
		nuit = !(Heure >= 10 && Heure < 22);
		tempLimite = nuit ? tempNuit : tempJour;
		minVentilo = nuit ? minVentiloNuit : minVentiloJour;

        // Aff heure
		int pourcentVentiloMin = round((float)minVentilo / 255 * 100);
        cout 	<< "\e[1m\e[92m[ " << to_string(Heure) << ":" << to_string(Minutes) + ":" << to_string(Secondes) << " ]\e[0m\e[92m"
				<< " : Temp max: " << to_string(tempLimite) << " °C : Ventilo min: " << to_string(pourcentVentiloMin) << " % \e[0m" << endl;

		// Pour chaque carte ..
		for (Carte &CarteA : Cartes) {
			int Temp = CarteA.getTemp();
			int Ventilo = CarteA.getVentilo();
			int nouvVentilo = Ventilo;

			// Vérif température
			if (Temp > tempCritique) {
				// TEMPERATURE CRITIQUE
				//system("pkill ethdcrminer64");
				CarteA.setVentilo(255);
				cout << "!!! Temperature critique !!!" << endl;
			} else {
				// EXECUTION NORMALE
				// Calcul delta
				int Ecart = Temp - tempLimite;
				if (Ecart < -4)
					Ecart = -4;
				Ecart = abs(Ecart);
				// Calcul pas
				int pas = round(Ecart / 2);
				if (pas == 0)
					pas = 1;

				// Calcul nouvelle vitesse ventilo
				string indicPas = "0";
				if (Temp > tempLimite) {
					// Augmentation du ventilo
					if (nouvVentilo + pas <= 255) {
						if (Ecart > 2)
							pas = pas * 2;
						nouvVentilo += pas;
					}
					// Indicateur
					indicPas = "+" + to_string(pas);
				} else if (Temp < tempLimite) {
					// Diminution du ventilo
					if (nouvVentilo-pas >= 0)
						nouvVentilo -= pas;
					// Indicateur
					indicPas = "-" + to_string(pas);
				}
				// Minimum ventilo
				if (nouvVentilo < minVentilo)
					nouvVentilo = minVentilo;

				// Application de la nouvelle vitesse si changement
				if (nouvVentilo != Ventilo)
					CarteA.setVentilo(nouvVentilo);

				// Status
				// \e[94m : Bleu
				// \e[1m  : Gras
				// \e[0m  : Reinit
				int pourcent = round((float)nouvVentilo / 255 * 100);
				cout << "\e[94m\e[1mCarte " + to_string(CarteA.getNum()) + ":\e[0m \e[94m"
					<< to_string(Temp) << "°C || "
					<< "Ventilo: " << to_string(pourcent) << "% (" << to_string(nouvVentilo) << ") || "
					<< "Pas: " << indicPas << "\e[39m"
				<< endl;
			}
		}
		// Délai
		sleep(2);
	}

	return 0;
}

// Vérif si dossier existe
bool is_dir(string Chemin) {
	return (stat(Chemin.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}
