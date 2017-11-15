class Carte {
private:
    // Proprietés
    int     Numero;
    // Chemins vers les fichiers moniteur
    string  fTemp,
            fVentilo;
    // Données en temps réel
    int     Temperature,
            Ventilo = 155; // Vitesse du ventilo (0 - 255)

public:
    // Constructeur
    Carte(int Num, string Moniteur) {
        this->Numero = Num;
        // Chemins
        this->fTemp     = Moniteur + "/temp1_input";
        this->fVentilo  = Moniteur + "/pwm1";

        // Init du ventilo avec la valeur par défaut
        this->setVentilo(this->Ventilo);
    }

    // Retourne le numéro
    int getNum() { return this->Numero; }
    // Retourne la vitesse du ventilo
    int getVentilo() { return this->Ventilo; }

    // Retourne la température actuelle de la carte
    int getTemp() {
        string valFtemp;
        int Retour;

        // Lecture de la température brute
        ifstream lectTemp(this->fTemp);
    	lectTemp >> valFtemp;
    	lectTemp.close();

        // Détermination de la température réelle en degrés
        int tempReelle = stoi(valFtemp) / 1000;
        // Ventilo 2 = Hashrate proportionnel à la vitesse du ventilo
        /*if (this->Numero == 3 && tempLimite > 70 && !nuit && modeBoost)
            tempReelle += 3;*/

        return tempReelle;
    }

    // Définition de la vitesse du ventilo
    void setVentilo (int vitesse) {
        // Commande hardware
    	ofstream lectVentilo(this->fVentilo, ios::trunc);
    	lectVentilo << vitesse;
    	lectVentilo.close();
        // Enregistrement
        this->Ventilo = vitesse;
    }
};
