#include "Affichage.h"
#include <iostream>
#include <string>
#include "ReseauMulticouche.h"
#include <fstream>
#include <algorithm>
#include <Windows.h>

#pragma execution_character_set( "utf-8" )

using namespace std;

int main()
{
    SetConsoleOutputCP(65001);
    cout<<"Bienvenue dans ce programme de compression d’image par reseau de neurones !" << endl ;
    cout<<"Voulez-vous :"<<endl<<"1 - Créer un nouveau reseau"<<endl<<"2 - Charger un reseau existant"<<endl<<"Entrez le chiffre correspondant a votre choix : ";
    int menu ;
    while ( ! ( cin >> menu ) || (menu < 1) || (menu > 2))
    {
        cout << "Vous devez entrer 1 ou 2. Recommencez : ";
        if ( cin.fail() )
        {
            cin.clear();
            cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        }
    }
    unsigned int prec;
    switch(menu)
    {
    case 1 :
        cout<< "Veuillez renseigner les paramètres du reseau :" <<endl ;
        //cout<<"Precision du calcul (32, 64 ou 80) : ";
        cout<<"Précision du calcul (32 ou 64) : ";
        while ( ! ( cin >> prec ) || (prec != 32 && prec != 64 /*&& prec != 80*/))
        {
            cout << "Vous devez entrer 32 ou 64, recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        switch (prec)
        {
        case 32:
            principale<float>();
            break;
        case 64:
            principale<double>();
            break;
      /*case 80:
            principale<long double>();
            break;*/
        }
        break ;
    case 2 :
        vector<string> chemins ;
        for (const auto & entree : filesystem::recursive_directory_iterator("."))
        {
            if(entree.path().extension() == ".rsn")
                chemins.push_back(entree.path().string().erase(0,2));
        }
        string fichier;
        cout<<"Donnez le nom du fichier de sauvegarde : ";
        while ( ! ( cin >> fichier ) || (find(chemins.begin(),chemins.end(),fichier) == chemins.end()))
        {
            cout << "Le fichier que vous avez indiqué n’est pas valide ou pas présent. Liste des fichiers disponibles :";
            for (size_t i=0 ; i<chemins.size() ; ++i)
            {
                cout<<endl<<chemins[i];
            }
            cout<<endl<<"Veuillez entrer un nom de fichier correct : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        ifstream flux(fichier.c_str());
        flux>>prec;
        switch (prec)
        {
        case 32:
            principale<float>(&flux,fichier);
            break;
        case 64:
            principale<double>(&flux,fichier);
            break;
      /*case 80:
            principale<long double>(&flux);
            break;*/
        }
        break ;
    }
    return 0;
}
