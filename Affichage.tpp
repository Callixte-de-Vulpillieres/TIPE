#include "Affichage.h"

template <typename T>
void principale(ifstream *iflux, string fichier)
{
    random_device rd;
    mt19937 generateur(rd());

    pair<unsigned int,unsigned int> taille=make_pair(0,0) ;
    ReseauMulticouche<T> reseau(&generateur);
    size_t num_image = 0;
    vector<T> distorsion, vgain ;

    if(iflux==nullptr)
    {
        cout<<"Largeur du reseau : ";
        while ( ! ( cin >> taille.first ) || taille.first < 1)
        {
            cout << "Vous devez entrer un nombre entier strictement positif, recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        cout<<"Hauteur du reseau : ";
        while ( ! ( cin >> taille.second ) || taille.second < 1)
        {
            cout << "Vous devez entrer un nombre entier strictement positif, recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        float taux_compression = 1;
        cout<<"Taux de compression : ";
        while ( ! ( cin >> taux_compression ) || taux_compression <= 0 )
        {
            cout << "Vous devez entrer un nombre decimal strictement positif, recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        unsigned int taille_compressee = max<unsigned int>(static_cast<unsigned int>(taille.first*taille.second*taux_compression),1);
        cout<<"La valeur retenue reelle est de "<< (static_cast<float>(taille_compressee)/static_cast<float>(taille.first*taille.second))<< " ."<<endl;
        unsigned int couches_cachees = 0;
        cout<<"Nombre de couches cachees : ";
        while ( ! ( cin >> couches_cachees ))
        {
            cout << "Vous devez entrer un nombre entier positif, recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }
        reseau.initialiser(taille,taille_compressee,couches_cachees,0.005);
    }
    else
    {
        char c;
        size_t t;
        T val1 , val2;
        (*iflux)>>t;
        for (size_t i=0 ; i<t ; ++i)
        {
            (*iflux)>>val1>>c>>val2;
            distorsion.push_back(val1);
            vgain.push_back(val2);
        }
        (*iflux)>>num_image;
        taille = reseau.charger(*iflux);
    }
    CImg<T> image;
    CImg<T> imagebis;
    vector<vector<T>> imagenor;
    for (unsigned int i=0 ; i<taille.first ; i++)
    {
        vector<T> ligne;
        for (unsigned int j=0 ; j<taille.second ; j++)
        {
            ligne.push_back(0);
        }
        imagenor.push_back(ligne);
    }

    //Chargement de la banque d'image
    vector<string> dataset;
    string chemin;
    cout<<"Dans quel dossier sont les images ? " ;
    while ( ! ( cin >> chemin ))
    {
        cout << "Vous devez entrer un nom de dossier : ";
        if ( cin.fail() )
        {
            cin.clear();
            cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        }
    }
    for (const auto & entree : filesystem::recursive_directory_iterator(chemin))
    {
        if(entree.path().extension() == ".png" || entree.path().extension() == ".bmp" )
            dataset.push_back(entree.path().string());
    }
    cout<<dataset.size()<< " images ont été detectées."<<endl;
    shuffle(begin(dataset),end(dataset),rd);
    bool afficherim = false, accesaleat = true, ignorernoir = false;
    while (true)
    {
        int menu;
        cout<<endl<<"Vous pouvez :"<<endl<<"1 - Entraîner sur un nombre d’images determiné"<<endl<<"2 - Entraîner pour un temps determiné"<<endl<<"3 - Afficher les statistiques d’entraînement"<<endl;
        if (afficherim)
        {
            cout<<"4 - Cesser d’afficher les images"<<endl;
        }
        else
        {
            cout<<"4 - Afficher les images"<<endl;
        }
        if(accesaleat)
        {
            cout<<"5 - Entraîner séquentiellement le reseau"<<endl;
        }
        else
        {
            cout<<"5 - Entraîner aléatoirement le reseau"<<endl;
        }
        if(!ignorernoir)
        {
            cout<<"6 - Ignorer les zones noires"<<endl;
        }
        else
        {
            cout<<"6 - Entraîner sur les zones noires"<<endl;
        }
        cout<<"7 - Modifier la valeur du gain"<<endl<<"8 - Enregistrer sous"<<endl;
        if(fichier != "")
        {
            cout<<"9 - Enregistrer"<<endl;
        }
        cout<<"10 - Passer en mode automatique"<<endl;
        cout<<"Entrez le chiffre corresondant a votre choix : ";
        while ( ! ( cin >> menu ) || (menu < 1) || (menu > 10) || (menu==9 && fichier==""))
        {
            cout << "Vous devez entrer un des chiffres proposés. Recommencez : ";
            if ( cin.fail() )
            {
                cin.clear();
                cin.ignore( numeric_limits<streamsize>::max(), '\n' );
            }
        }

        auto sauvegarder = [&reseau,&distorsion,&vgain,&num_image](ofstream &flux) -> void
        {
            flux<<8*static_cast<unsigned int>(sizeof(T))<<endl<<distorsion.size()<<endl;
            for (size_t i = 0 ; i<distorsion.size() ; ++i)
            {
                flux<<distorsion[i]<<","<<vgain[i]<<endl;
            }
            flux<<num_image<<endl;
            reseau.sauvegarder(flux);
        };

        auto gereimage = [&] () -> void
        {
                image.load(dataset[num_image%dataset.size()].c_str());
                image.channel(0);
                image *=(255.0/BLANC);
                if(afficherim)
                {
                    imagebis.assign(image.width(),image.height()) ;
                }
                vector<pair<unsigned int,unsigned int>> decoupage ;
                for (unsigned int k=0 ; k<=image.width()-taille.first ; k+=taille.first)
                {
                    for(unsigned int h=0 ; h<=image.height()-taille.second ; h+=taille.second)
                    {
                        decoupage.push_back(make_pair(k,h));
                    }
                    if (image.height()%taille.second != 0)
                    {
                        decoupage.push_back(make_pair(k,image.height()-taille.second));
                    }
                }
                if (image.width()%taille.first != 0)
                {
                    for(unsigned int h=0 ; h<=image.height()-taille.second ; h+=taille.second)
                    {
                        decoupage.push_back(make_pair(image.width()-taille.first,h));
                    }
                    if (image.height()%taille.second != 0)
                    {
                        decoupage.push_back(make_pair(image.width()-taille.first,image.height()-taille.second));
                    }
                }
                if (accesaleat)
                {
                    shuffle(begin(decoupage),end(decoupage),rd);
                }
                for (unsigned int k=0 ; k<decoupage.size() ; k++)
                {
                    bool noir = ignorernoir;
                    for (unsigned int i=0 ; i<taille.first ; i++)
                    {
                        for (unsigned int j=0 ; j<taille.second ; j++)
                        {
                            imagenor[i][j]=image(decoupage[k].first+i,decoupage[k].second+j)/127.5 -1.0;
                            noir = noir && (imagenor[i][j]==-1);
                        }
                    }
                    if(!noir)
                    {
                        reseau.set_entree(imagenor);
                        distorsion.push_back(reseau.get_distorsion());
                        vgain.push_back(reseau.get_gain());
                        reseau.gen();
                    }
                    if(afficherim)
                    {
                        auto sortie = reseau.get_sortie();
                        for (unsigned int i=0 ; i<taille.first ; i++)
                        {
                            for (unsigned int j=0 ; j<taille.second ; j++)
                            {
                                imagebis(decoupage[k].first+i,decoupage[k].second+j)= noir ? 0 : sortie[i][j]*255.0/BLANC;
                            }
                        }
                    }
                }
                if(afficherim)
                {
                    CImgDisplay fen1(image,"Original",0), fen2(imagebis,("Compressee. MSE : " + to_string(image.MSE(imagebis)/(255.0*255.0))).c_str(),0);
                    while (!fen1.is_closed() && !fen2.is_closed())
                    {
                      fen1.wait();
                    }
                }
                ++num_image;
        };

        switch (menu)
        {
        case 1 :
            {
                unsigned int objectif;
                cout<<"Combien ? " ;
                while ( ! ( cin >> objectif ))
                {
                    cout << "Vous devez entrer un entier strictement positif. Recommencez : ";
                    if ( cin.fail() )
                    {
                        cin.clear();
                        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
                    }
                }
                for (unsigned int l=0 ; l<objectif ; l++)
                {
                    cout<<'\r'<<"                                                                    "<<'\r';
                    cout<<"Image "<<l+1<<"/"<<objectif;
                    gereimage();
                }
                cout<<endl;
            }
            break;
        case 2 :
            {
                char l;
                unsigned int heures,minutes;
                cout<<"Combien de temps (hh:mm) ? " ;
                while ( ! ( cin >> heures>>l>>minutes ) || (minutes>59))
                {
                    cout << "Vous devez entrer une durée de la forme hh:mm, par exemple \"01:30\". Recommencez : ";
                    if ( cin.fail() )
                    {
                        cin.clear();
                        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
                    }
                }
                time_t debut = time(NULL);
                unsigned int p=0;
                while(difftime(time(NULL),debut) < 60*(60*heures+minutes))
                {
                    p++;
                    cout<<'\r'<<"                                                                    "<<'\r';
                    cout<<"Image "<<p<<". Progression : "<<difftime(time(NULL),debut)/(0.6*(60*heures+minutes))<<"% .";
                    gereimage();
                }
                cout<<endl;
            }
            break;
        case 3 :
            {
                unsigned int resolution = min(1000u,static_cast<unsigned int>(distorsion.size()));
                CImg<T> valdist(1,resolution,1,1,0);
                CImg<T> valgain(1,resolution,1,1,0);
                double a = 1.0/static_cast<double>(resolution);
                for (unsigned int i=0; i<resolution ; i++)
                {
                    for(size_t j=static_cast<size_t>(a*static_cast<double>(distorsion.size()*i)); j<static_cast<size_t>(a*static_cast<double>(distorsion.size()*(i+1))) ; j++)
                    {
                        valdist(0,i) += distorsion[j] ;
                        valgain(0,i) += vgain[j] ;
                    }
                    valdist(0,i) /= static_cast<size_t>(a*static_cast<double>(distorsion.size()*(i+1)))-static_cast<size_t>(a*static_cast<double>(distorsion.size()*i));
                    valgain(0,i) /= static_cast<size_t>(a*static_cast<double>(distorsion.size()*(i+1)))-static_cast<size_t>(a*static_cast<double>(distorsion.size()*i));
                }             
                cout<<"Approximtion de la limite : "<<accumulate(distorsion.end()-distorsion.size()/5,distorsion.end(),0.0l)/static_cast<long double>(distorsion.size()/5)<<endl;
                unsigned int sec = static_cast<unsigned int>(reseau.get_duree('c') + reseau.get_duree('d') + reseau.get_duree('e'));
                cout<<"Durée totale de travail : "<<sec/3600<<"h"<<(sec%3600)/60<<"mn"<<sec%60<<"s, pour un total de "<<distorsion.size()<<" itérations, sur "<<num_image<<" images."<<endl;
                cout<<"Vitesse de compression : "<<reseau.get_duree('c')/(distorsion.size()*taille.first*taille.second)<<" s/px ."<<endl;
                cout<<"Vitesse de décompression : "<<reseau.get_duree('d')/(distorsion.size()*taille.first*taille.second)<<" s/px ."<<endl;
                cout<<"Vitesse de rétropropagation : "<<reseau.get_duree('e')/(distorsion.size()*taille.first*taille.second)<<" s/px ."<<endl;
                valdist.display_graph("Distortion au cours de l'entrainement",1,1,"Nombre d'iterations",0,distorsion.size(),"MSE");
                valgain.display_graph("Gain au cours de l'entrainement",1,1,"Nombre d'iterations",0,distorsion.size(),"Gain");
            }
            break;
        case 4 :
            afficherim = !afficherim ;
            break;
        case 5 :
            accesaleat = !accesaleat ;
            break;
        case 6 :
            ignorernoir = !ignorernoir;
            break;
        case 7 :
            {
                T gain = reseau.get_gain();
                cout<<"Quelle valeur ? (0 pour conserver la valeur actuelle ("<<gain<<")) : ";
                while ( ! ( cin >> gain ))
                {
                    cout << "Vous devez entrer un nombre décimal positif, recommencez : ";
                    if ( cin.fail() )
                    {
                        cin.clear();
                        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
                    }
                }
                reseau.set_gain(gain);
            }
            break;
        case 8 :
            {
                vector<string> chemins ;
                for (const auto & entree : filesystem::recursive_directory_iterator("."))
                {
                    if(entree.path().extension() == ".rsn")
                        chemins.push_back(entree.path().string());
                }
                string fichier2;
                cout<<"Donnez le nom du fichier de sauvegarde, en .rsn ou il ne pourra pas être lu : ";
                while ( ! ( cin >> fichier2 ) || (find(chemins.begin(),chemins.end(),fichier2) != chemins.end()))
                {
                    cout<<endl<<"Veuillez entrer un nom de fichier correct, non déjà présent : ";
                    if ( cin.fail() )
                    {
                        cin.clear();
                        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
                    }
                }
                fichier=fichier2;
                ofstream oflux(fichier.c_str());
                sauvegarder(oflux);
            }
            break;
        case 9 :
            {
                ofstream oflux(fichier.c_str());
                sauvegarder(oflux);
            }
            break;
        case 10 :
            {
                time_t temps = time(NULL);
                cout<<"L’entraînement automatique du réseau est en cours, veuillez patienter..."<<endl;
                afficherim = false;
                while (distorsion.size()<5000000)
                {
                    gereimage();
                    cout<<"                                                                           \rItérations : "<<distorsion.size()<<"/5000000 .\r";
                }
                cout<<"                                                                                                        \r";
                long double lim = accumulate(distorsion.end()-distorsion.size()/5,distorsion.end(),0.0l)/static_cast<long double>(distorsion.size()/5);
                bool gain_modifie = true;
                long double comp = accumulate(distorsion.end()-2*(distorsion.size()/5),distorsion.end()-distorsion.size()/5,0.0l)/accumulate(distorsion.end()-distorsion.size()/5,distorsion.end(),0.0l);
                if (fichier != "")
                {
                    ofstream oflux(fichier.c_str());
                    sauvegarder(oflux);
                }
                while (gain_modifie || comp>1.001)
                {
                    size_t objectif = (10*distorsion.size())/9;
                    while(distorsion.size()<objectif)
                    {
                        gereimage();
                        cout<<"\rLimite : "<<lim;
                        cout<<". Itérations : "<<distorsion.size()<<"/"<<objectif <<". Progression : "<<comp<<"                       ";
                    }
                    comp = accumulate(distorsion.end()-2*(distorsion.size()/5),distorsion.end()-distorsion.size()/5,0.0l)/accumulate(distorsion.end()-distorsion.size()/5,distorsion.end(),0.0l);
                    lim = accumulate(distorsion.end()-distorsion.size()/5,distorsion.end(),0.0l)/static_cast<long double>(distorsion.size()/5);
                    if(comp<=1.001 && (!gain_modifie))
                    {
                        reseau.set_gain(reseau.get_gain()*0.7);
                        gain_modifie=true;
                    }
                    else
                    {
                        gain_modifie = false ;
                    }
                    if (fichier != "")
                    {
                        ofstream oflux(fichier.c_str());
                        sauvegarder(oflux);
                    }
                }
                temps = time(NULL)-temps;
                cout<<endl<<"Entraînement terminé, temps d'execution réel : "<<temps/3600<<"h"<<(temps%3600)/60<<"mn"<<temps%60<<"s .                        "<<endl;
            }
        }
    }
}
