#ifndef RESEAUMULTICOUCHE_H
#define RESEAUMULTICOUCHE_H

#include <utility>
#include <vector>
#include <random>
#include <math.h>
#include <string>
#include <fstream>
#include <ctime>
#include "Affichage.h"

using namespace std;

float transfert(float x);
double transfert(double x);
long double transfert(long double x);



template <class T> //T peut être float, double ou long double, en fonction de la précision souhaitée
class ReseauMulticouche
{
    public:
    ReseauMulticouche(mt19937 *generateur) ;
    void initialiser(pair<unsigned int,unsigned int> taille_entree, unsigned int taille_compresse, unsigned int couches_cachees,T gain);
    pair<unsigned int,unsigned int> charger(ifstream& flux);
    void set_entree(const vector<vector<T>> &entree);
    vector<vector<T>> get_sortie();
    void calcsortie(); //Calcul la sortie du réseau
    T get_distorsion();
    void gen(); //effectue les modifications nécessaires sur le réseau en fonction de la distorsion
    void set_gain(T gain);
    T get_gain();
    void sauvegarder(ofstream& flux);
    double get_duree(char c);

    private:
    pair<unsigned int, unsigned int> m_taille_entree ;
    vector<vector<pair<T,vector<T>>>> m_reseau ;
    vector<vector<T>> m_y;
    vector<unsigned int> m_taille_couche ;
    vector<T> m_entree;
    T m_distorsion;
    mt19937* m_generateur;
    bool m_csortie=false;
    bool m_cdistorsion=false;
    T m_gain;
    double m_compression = 0 ;
    double m_decompression = 0;
    double m_entrainement = 0;
};

#include "ReseauMulticouche.tpp"
#endif // RESEAUMULTICOUCHE_H
