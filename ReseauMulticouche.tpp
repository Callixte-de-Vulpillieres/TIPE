#include "ReseauMulticouche.h"

template <class T>
void ReseauMulticouche<T>::initialiser(pair<unsigned int,unsigned int> taille_entree, unsigned int taille_compresse, unsigned int couches_cachees, T gain)
{
    m_gain = gain;
    m_taille_entree = make_pair(taille_entree.first , taille_entree.second);
    unsigned int te = taille_entree.first*taille_entree.second;
    m_entree = vector<T> (te,0);

    /*Détermination de la taille de chaque couche. Le profil du réseau est en  >< , avec des segments les plus linéaires possibles.*/
    for (unsigned int k=0 ; k<2*couches_cachees+3;k++) //pour chaque couche
    {
        if (k<=couches_cachees+1) //si l'on est avant la couche compressée
            m_taille_couche.push_back(te - static_cast<unsigned int>(static_cast<float>((te - taille_compresse)*k)/static_cast<float>(couches_cachees+1)));
        else
            m_taille_couche.push_back(te - static_cast<unsigned int>(static_cast<float>((te - taille_compresse)*(2*couches_cachees +2 -k))/static_cast<float>(couches_cachees+1)));
    }

    /*initialisation du reseau en lui-même */
    uniform_real_distribution<T> distrib(-1,1);
    for (unsigned int k=0;k<2*couches_cachees+3;k++)
    {
        vector<pair<T,vector<T>>> couche;
        for (unsigned int i=0;i<m_taille_couche[k];i++)//pour chaque neurone de cette couche
        {
            vector<T> liaisons;
            liaisons.push_back(distrib(*m_generateur));
            if(k!=0) //si on n'est pas sur la première couche (où il n'y a pas de parents)
            {
                for (unsigned int j=0;j<m_taille_couche[k-1];j++)
                {
                    liaisons.push_back(distrib(*m_generateur));
                }
            }
            else
            {
                liaisons.push_back(distrib(*m_generateur));
            }
            couche.push_back(make_pair(static_cast<T>(0),liaisons));
        }
        m_reseau.push_back(couche);
        m_y.push_back(vector<T>(m_taille_couche[k],0));
    }
}

template <class T>
void ReseauMulticouche<T>::set_entree(const vector<vector<T> > &entree)
{
    for (unsigned int i=0 ; i<m_taille_entree.first ; i++)
    {
        for (unsigned int j=0 ; j<m_taille_entree.second ; j++)
        {
            m_entree[i*m_taille_entree.second + j] = entree[i][j];
        }
    }
    m_cdistorsion=false;
    m_csortie=false;
}

template <class T>
vector<vector<T>> ReseauMulticouche<T>::get_sortie()
{
    if (!m_csortie)
        calcsortie();
    vector<vector<T>> res;
    for (unsigned int i=0 ; i<m_taille_entree.first ; i++)
    {
        vector<T> ligne;
        for (unsigned int j=0 ; j<m_taille_entree.second ; j++)
        {
            ligne.push_back(round((m_reseau[m_reseau.size()-1][i*m_taille_entree.second + j].first+1)*BLANC/2));
        }
        res.push_back(ligne);
    }
    return res ;
}

template <class T>
void ReseauMulticouche<T>::calcsortie()
{
    clock_t d = clock();
    /* première étape : determiner la première couche cachée */
    for (unsigned int i = 0; i<m_reseau[0].size() ; i++) //Pour chaque neurone de la première couche
    {
        m_reseau[0][i].first = transfert(m_entree[i]*m_reseau[0][i].second[1] + m_reseau[0][i].second[0]);
    }

    /* second étape : la même chose sur toutes les couches */
    for (unsigned int k=1; k<=m_reseau.size()/2 ; ++k) //Pour chaque couche k de compreesion
    {
        for (unsigned int i = 0; i<m_reseau[k].size() ; i++) //Pour chaque neurone i de la couche k
        {
            T somme = m_reseau[k][i].second[0];
            for (unsigned int j=0 ; j<m_reseau[k-1].size() ; j++) //Pour chaque neurone j de la couche k-1
            {
                somme += m_reseau[k-1][j].first * m_reseau[k][i].second[j+1] ;
            }
            m_reseau[k][i].first = transfert(somme);
        }
    }

    //étape de quantification
    for(unsigned int i=0 ; i<m_reseau[m_reseau.size()/2 + 1].size() ; ++i)
    {
        m_reseau[m_reseau.size()/2 + 1][i].first = round((m_reseau[m_reseau.size()/2 + 1][i].first +1)*BLANC/2)*2/BLANC - 1;
    }


    m_compression += static_cast<double>((clock()-d))/CLOCKS_PER_SEC ;
    d=clock();
    for (unsigned int k=m_reseau.size()/2 + 1; k<m_reseau.size() ; k++) //Pour chaque couche k de décompreesion
    {
        for (unsigned int i = 0; i<m_reseau[k].size() ; i++) //Pour chaque neurone i de la couche k
        {
            T somme = m_reseau[k][i].second[0];
            for (unsigned int j=0 ; j<m_reseau[k-1].size() ; j++) //Pour chaque neurone j de la couche k-1
            {
                somme += m_reseau[k-1][j].first * m_reseau[k][i].second[j+1] ;
            }
            m_reseau[k][i].first = transfert(somme);
        }
    }
    m_decompression += static_cast<double>((clock()-d))/CLOCKS_PER_SEC ;
    m_csortie =true;
}

template <class T>
T ReseauMulticouche<T>::get_distorsion()
{
    if (!m_csortie)
        calcsortie();
    if (!m_cdistorsion)
    {
        T somme = 0;
        for (unsigned int i=0 ; i<m_entree.size() ; i++)
        {
            somme += pow((m_entree[i]+1)/2-round((m_reseau[m_reseau.size()-1][i].first+1)*BLANC/2)/BLANC,2);
        }
        m_distorsion = somme/static_cast<T>(m_entree.size()) ;
        m_cdistorsion = true;
    }
    return m_distorsion ;
}

template <class T>
void ReseauMulticouche<T>::gen()
{
    if(!m_csortie)
        calcsortie();
    clock_t d = clock();
    vector<vector<T>> y;

    /* On commence par calculer y pour chaque neurone
    On a pour fonction de transfert f(x)=(exp(x)-1)/(exp(x)+1)
    On remarque que l'on a : f'(x) = 0.5 * (1-f(x)^2) */

    size_t k=m_reseau.size()-1; //couche du calcul en cours
    for (unsigned int i=0 ; i<m_reseau[k].size() ; i++) //Couche de sortie
    {
        m_y[k][i]=(1-(m_reseau[k][i].first)*(m_reseau[k][i].first))*(m_reseau[k][i].first - m_entree[i]);
    }

    for (int k=m_reseau.size()-2 ; k>=0 ; k--)//pour chaque couche k
    {
        for (unsigned int i=0 ; i<m_reseau[k].size() ; i++)//pour chaque neurone i de cette couche
        {
            T somme = 0;
            for (unsigned int j=0 ; j<m_reseau[k+1].size() ; j++) //Pour chaque neurone fils j
            {
                somme+=m_y[k+1][j]*m_reseau[k+1][j].second[i+1];
            }
            m_y[k][i]=static_cast<T>(0.5*(1-(m_reseau[k][i].first)*(m_reseau[k][i].first)))*somme;
        }
    }

    /*On modifie maintenant les poids*/
    for(k=1 ; k<m_reseau.size() ; k++)
    {
        for(unsigned int i=0 ; i<m_reseau[k].size() ; i++)
        {
            m_reseau[k][i].second[0] -= m_gain*m_y[k][i] ;
            for(unsigned int j=1 ; j<m_reseau[k][i].second.size() ; j++)
            {
                m_reseau[k][i].second[j] -= m_gain*m_y[k][i]*m_reseau[k-1][j-1].first;
            }
        }
    }
    for(unsigned int i=0 ; i<m_reseau[0].size() ; i++)
    {
        m_reseau[0][i].second[0] -= m_gain*m_y[0][i] ;
        m_reseau[0][i].second[1] -= m_gain*m_y[0][i]*m_entree[i];
    }
    m_entrainement += static_cast<double>((clock()-d))/CLOCKS_PER_SEC ;

}

template <class T>
void ReseauMulticouche<T>::set_gain(T gain)
{
    m_gain = 0.1>gain && gain>0 ? gain : m_gain;
}

template <class T>
void ReseauMulticouche<T>::sauvegarder(ofstream& flux)
{
    flux<<m_taille_entree.first<<","<<m_taille_entree.second<<endl;
    flux<<m_taille_couche.size()<<endl ;
    for(size_t k = 0 ; k < m_taille_couche.size() ; ++k)
    {
        flux<<m_taille_couche[k]<<endl ;
    }
    for(size_t h=0 ; h<m_taille_couche[0]; ++h)
    {
        flux<<m_reseau[0][h].second[0]<<","<<m_reseau[0][h].second[1]<<endl;
    }
    for (size_t k=1 ; k<m_taille_couche.size() ; ++k)
    {
        for(size_t h=0 ; h<m_taille_couche[k]; ++h)
        {
            flux<<m_reseau[k][h].second[0]<<endl;
            for(size_t l=1 ; l<=m_taille_couche[k-1] ; ++l)
            {
                flux<<m_reseau[k][h].second[l]<<endl;
            }
        }
    }
    flux<<m_gain<<endl;
    flux<<m_compression<<","<<m_decompression<<","<<m_entrainement<<endl;
}

template <class T>
ReseauMulticouche<T>::ReseauMulticouche(mt19937* generateur)
{
    m_generateur = generateur ;
}

template <class T>
pair<unsigned int,unsigned int> ReseauMulticouche<T>::charger(ifstream &flux)
{
    char c,cc;
    unsigned int i;
    unsigned int j;
    size_t taille ;
    T val1 ;
    T val2 ;
    flux>>i>>c>>j;
    m_taille_entree = make_pair(i,j);
    flux>>taille;
    for (size_t k=0 ; k<taille ; ++k)
    {
        flux>>i;
        m_taille_couche.push_back(i);
    }

    //première couche
    vector<pair<T,vector<T>>> couche ;
    for(size_t h=0 ; h<m_taille_couche[0]; ++h)//pour chaque neurone de cette couche
    {
        vector<T> liaisons ;
        flux>>val1>>c>>val2;
        liaisons.push_back(val1);
        liaisons.push_back(val2);
        couche.push_back(make_pair(0,liaisons));
    }
    m_reseau.push_back(couche);
    m_y.push_back(vector<T>(m_taille_couche[0],0));
    for (size_t k=1 ; k<m_taille_couche.size() ; ++k) //pour chaque couche
    {
        vector<pair<T,vector<T>>> couche ;
        for(size_t h=0 ; h<m_taille_couche[k]; ++h) //pour chaque neurone
        {
            vector<T> liaisons ;
            flux>>val1;
            liaisons.push_back(val1);
            for(size_t l=1 ; l<=m_taille_couche[k-1] ; ++l) //pour chaque liaison
            {
                flux>>val1;
                liaisons.push_back(val1);
            }
            couche.push_back(make_pair(0,liaisons));
        }
        m_reseau.push_back(couche);
        m_y.push_back(vector<T>(m_taille_couche[k],0));
    }
    flux>>m_gain;
    flux>>m_compression>>c>>m_decompression>>cc>>m_entrainement;
    m_entree = vector<T>(m_taille_entree.first*m_taille_entree.second,0);
    return m_taille_entree;
}

template <class T>
T ReseauMulticouche<T>::get_gain()
{
    return m_gain;
}

template <class T>
double ReseauMulticouche<T>::get_duree(char c)
{
    switch(c)
    {
        case 'e' :
            return m_entrainement;
        case 'c' :
            return m_compression;
        default :
            return m_decompression;
    }
}

float transfert(float x)
{
    return tanh(0.5f*x);
}

double transfert(double x)
{
    return tanh(0.5*x);
}

long double transfert(long double x)
{
    return tanh(0.5l*x);
}
