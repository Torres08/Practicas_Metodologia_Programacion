/*
 * File:   main.cpp
 * Author: MP-team 
 * @warning To be implemented by students. Follow the instructions given in the headers
 */

/**
 * @brief This program must load a sample dataset given as parameters to the program.
 * 
 * The main() function has three mandatory parameters:  
 * "-input <input_file>"  "-events <number_events>"  "-K <numberOfRecommendations>"
 *    - input_file is the path to the file used as input
 *    - number_events is the number of events to be read from the input_file
 *    - K is the number of recommendations
 * there are also three optional parameters:
 * "-type purchase|cart|view|remove_from_cart"  "-output <output_file>"  "-display Framed|Fancy|Plain"
 *    - type  marks the type used for create the brand relationships
 *  *  * - If the parameter is absent, it is considered purchase as default
 *    - output_file is the file to store the output
 *  *  * - If the parameter is absent, input_file+".pgrk" is used
 *    - display is used to choose the format of the DataTable output:
 *  *  * - If the parameter is absent, the report is shown in plain text, using DataTable::showPlainReportH(0)
 *  *  * - If the parameter is set to Framed, the report is shown using a table format, with DataTable::showFramedReportH(0)
 *  *  * - If the parameter is set to Fancy, the report is shown as histograms using an external library, with DataTable::showFancyReportH(0)
 * 
 * ALL THE PARAMETERS CAN BE GIVEN IN ANY ORDER
 *  
 */

/**
 * NOTE: NO PARAMETERS EXECUTION
 * In order to modularize the development of this assignment, a simplified execution
 * mode is proposed, where only one parameter is employed. The execution of the
 * program using:
 * 
 * shopping5 -noargs
 * 
 * is intended to be used with input redirection. 
 * 
 * The proper way to call the program using this structure is the following (using
 * ECommerce30.keyboard as input, for example)
 * dist/Debug/GNU-Linux/shopping5 -noargs < tests/validation/input_data
 */

#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include "Event.h"
#include "EventSet.h"
#include "Index.h"
#include "DataVector.h"
#include "MPTests.h"
#include "Matrix.h"

#define ERROR_ARGUMENTS 1
#define ERROR_OPEN 2
#define ERROR_DATA 3
#define MAXBRANDS 5000

#define ALFA 0.85
#define MAX_ITER 200

using namespace std;

/****************************************************************/
/***************  GIVEN AND IMPLEMENTED METHODS  ****************/
/****************************************************************/

/**
 * @brief Breaks the program execution if an error is encountered during execution
 * @param errorcode Integer marking the type of error encountered (1 for arguments
 * error, 2 for open file error and 3 for data reading error, see ERROR_ARGUMENTS,
 * ERROR_OPEN and ERROR_DATA definitions above)
 * @param errordata String representing where the error has been produced (mainly
 * used to indicate the file that produced the error)
 */
void errorBreak(int errorcode, const string & errordata);

/**
 * @brief Helper function that prints information about main parameters when the
 * program is not called properly. Should be called from @see errorBreak when the
 * errorcode is equal to 1
 */
void showHelp();
  
/**
 * Find the position pointing to the less value in V
 * @param V DataVector
 * @return position of the less value
 */
int getLower(const DataVector &V) ;

/**
 * @brief Select the K columns in the matrix pr (it is assumed that pr has 1 x n 
 * dimmensions) with higher values. 
 * If the matrix has less than K columnx, all the columns in the matrix are selected
 * @param pr matrix that is labeled
 * @param [in|out] k number of recommendations shown 
 * @return DataVector with the best recommendations
 * @return K number of values selected
 */
DataVector selectKbest(const Matrix & pr, int &k);

/**
 * @brief Compute the set of unique Brands in those events pointed by the index idx, 
 * filtering (no selecting) the unknown brand (event.getBrand()=="")
 * @param evs EvenSet where the evenst are stored
 * @param idx Index whith the relevant events
 * @param [in|out] uniqueBrands  The set of uniqueBrands (it is assumed that has enough memory allocated)
 * @param [in|out] n_values number of unique brands
 * @pre It is assumed that uniqueBrands has enoung memory allocated to store all the unique brands
 */
void findUniqueBrandsInIndex(const EventSet & evs, const Index & idx, string * uniqueBrands, int & n_values);

/**
 * @brief Process the main input parameters
 * @param argc
 * @param argv
 * @param ifilename
 * @param display (default plain)
 * @param rel_type (dafault purchase)
 * @param K
 * @param number_events
 * @param ofilename (default ifilename+".pgrk")
 * @return true if the parameters are correct, false otherwise
 */
bool processInputParameters(int argc, char **argv, string & ifilename, string & display,string & rel_type, int &K, int & number_events,string &ofilename );

/**
 * @brief Read the input parameters from the input stream
 * @param ifilename
 * @param display
 * @param rel_type
 * @param K
 * @param number_events
 * @param ofilename
 */
void readInputParameters(string & ifilename, string & display,string & rel_type, int &K, int & number_events,string &ofilename );

/**
 * @brief Do Shopping5 task
 * @param argc number of input parameters
 * @param argv the set of parameters as c-string
 */
void shopping5(int argc, char **argv);

/****************************************************************/
/*************  METHODS TO BE IMPLEMENTED BY STUDENTS  **********/
/****************************************************************/

/**
 * @brief Auxiliar function to compute the PageRank given a matrix and the rest of parameters
 * @param M 2D matrix
 * @param max_iter maximum number of iterations (stop condition)
 * @param threshold, stop condition for convergence
 * @param n, number of convergence values shown
 * @return PageRank values stores in a Matrix object (with size 1 row and M.columns() columns)
 */
Matrix PageRank(const Matrix & M, int max_iter, double threshold, int n){
    Matrix PRold(1, M.columns(), 1/(double)M.columns()), PRnew;
    double diff = threshold, add;
    bool stable = false;
    for (int i = 0; i < max_iter && !stable; i++){
        cout << "\nPageRank at iteration " << i << endl << PRold;
        PRnew = PRold * M;
        diff = 0;
        for (int j = 0; j < M.columns(); j++){
            add = PRnew.at(0,j) - PRold.at(0,j);
            if (add < 0)
                add *= -1;
            diff += add;
        }
        if (diff < threshold)
            stable = true;
        PRold = PRnew;
        n=i;
    }
    cout << "\nConvergence reached in " << n+1 << " iterations (stability last value = " << diff << ")\n";
    return PRnew;
}

/**
 * @brief Obtains a 1 x M.columns() matrix representing the PageRank values for each element
 * @param [in] M the adjacency matrix [out] Matrix with teletransportation
 * @param max_iter maximun number of iterations for PageRank algorithm
 * @param threshold used to stop the iterations (if the sum of the absolute differences between values in two consecutive iterations is less than a threshold the algorithm stop)
 * @param n number of convergence values shown
 * @return PageRank values stores in a Matrix object (with size 1 row and M.columns() columns)
 */
Matrix computePageRank(Matrix & M, int max_iter, double threshold, int n = 10) {
    Matrix pagerank;
    double alfa = ALFA;
    M.normalize();
    M = alfa * M + (1 - alfa) / M.columns();
    cout << "\nAfter teletransportation:\n" << M << endl;
    pagerank = PageRank(M, max_iter, threshold, n);
    return pagerank;
}

/**
 * @brief This method creates the adjacency matrix, i.e,
 * set m_adj("brandA","brandB") = k , being k the number of relationships between brands
 * set m_adj("brandA","brandB") = 0 otherwhise 
 * Note that the output matrix is be simmetric
 * @param evs The eventset 
 * @param idx The index sorted by sessions pointing to the events with proper type 
 * @param [in|out] m_adj it is assumend that the matrix has been previosly created
 * @prec  m_adj it is assumend that the matrix m_adj has been previosly created
 */
void createAdjacencyMatrix(const EventSet &evs, const Index & idx, Matrix & m_adj) {
    int lb = 0, ub = 0;
    string session;
    string brand1, brand2;
    for (int i = 0; i < idx.size();) {
        session = idx.at(i).getKey();
        ub = idx.upper_bound(session);
        i = ub; // This avoids repeating the algorithm for the same session id
        lb = idx.lower_bound(session);
        for (int left = lb; left < ub; left++) {
            for (int right = lb; right < left; right++) {
                if (left != right) {
                    brand1 = getEvent(evs, idx, left).getBrand();
                    brand2 = getEvent(evs, idx, right).getBrand();
                    if (brand1 != brand2 && getEvent(evs, idx, left + lb).getType() == getEvent(evs, idx, right + lb).getType() && brand1 != "" && brand2 != "") {
                        m_adj(brand1, brand2)++;
                        m_adj(brand2, brand1)++;
                    }
                }
            }
        }
    }
    // For each sesion in idx find the lower and upper bound positions in the index and 
    // compute all the relationships in this range
    // Every time a new relationships is obtained between two brands we increment in one 
    // i.e., m_adj("brandA","brandB")+=1;
}

/**
 * @brief Main block of the program. The full explanation of the behavior is given 
 * in the comment above
 * @param argc Number of arguments passed to main
 * @param argv Strings representing those arguments
 * @return 
 */
int main(int argc, char** argv) {   
    shopping5(argc, argv);  

//    Matrix xd(10,10,49.12);
    
    
    
//    cout << xd;
// valgrind --leak-check=full ./dist/Debug/GNU-Linux/shopping3a -input ./data/ECommerce162.dataset -K 5 -type cart -events 10000 -display Framed
    
}

/****************************************************************/
/***************  GIVEN AND IMPLEMENTED METHODS  ****************/
/****************************************************************/

void errorBreak(int errorcode, const string & errordata) {
    switch (errorcode) {
        case ERROR_ARGUMENTS:
            CVAL << endl << "Error in call" << endl;
            showHelp();
            break;
        case ERROR_OPEN:
            CVAL << endl << "Error opening file " << errordata << endl;
            break;
        case ERROR_DATA:
            CVAL << endl << "Data error in file " << errordata << endl;
            break;
    }
    std::exit(1);
}

void showHelp() {
    cout << "Please use: -input <filename> -events <number_events> -K <number> [-display <Fancy|Framed|Plain> -output <ofilename>]" << endl;
    cout << "\n-input <filename>\n\tRead events from dataset in <filename>";
    cout << "\n-events <number_events>\n\tMaximum number of events to read from dataset filename";
    cout << "\n-K <number>\n\tNumber of recommendations";
    cout << "\n-type  <purchase|view|cart|remove_from_cart>\n\tUsed to define the relationships (defaul purchase)";
    cout << "\n-display  <Fancy|Framed|Plain> \n\tDisplay mode (default Plain)";
    cout << "\n-output <ofilename> \n\t If no file is given ofilename =ifilename+\"pgrk\" ";
}

int getLower(const DataVector &V) {
    int posmenor = 0;
    double menor = V.getValue(0);
    int n = V.size();
    for (int i = 1; i < n; i++) {
        if (menor > V.getValue(i)) {
            menor = V.getValue(i);
            posmenor = i;
        }
    }
    return posmenor;
}
 
DataVector selectKbest(const Matrix & pr, int &k) {
    string *allLabels = new string[pr.columns()], strAux;
    pr.getColLabels(allLabels);
    
    double *allValues = new double[pr.columns()], dAux;
    for (int i = 0; i < pr.columns(); i++)
        allValues[i] = pr(0, i);
    
    Index values;
    Pair aux;
    for (int i = 0; i < pr.columns(); i++) {
        aux.set(to_string(allValues[i]), i);
        values.add(aux);
    }   
    
    DataVector V;
    int n = (k <= pr.columns()) ? k : pr.columns(), pos;
    V.alloc(n);
    for (int i = 0; i < n; i++){
        pos = values.at(pr.columns()-1-i).getPos();
        V.setLabel(i, allLabels[pos]);
        V.setValue(i, allValues[pos]);
    }
    
    delete[]allLabels;
    delete[]allValues;
    return V; 
}

void findUniqueBrandsInIndex(const EventSet & evs, const Index & idx, string *uniqueBrands, int & n_values) {
    Index idxUnique;
    string marca;
    n_values = 0;
    // Hint: It can be used an index to store only one brand (in the case the brand is included in the index we do not include it again)
    for (int i = 0; i < idx.size(); i++) {
        marca = evs.at(idx.at(i).getPos()).getBrand();
        if (marca != "") { // filter unknown brands 
            if (idxUnique.lower_bound(marca) == idxUnique.upper_bound(marca)) {
                idxUnique.add(marca);
                uniqueBrands[n_values] = marca;
                n_values++;
            }
        }
    }
}

bool processInputParameters(int argc, char **argv, string & ifilename, string & display,string & rel_type, int &K, int & number_events,string &ofilename ){
      bool noargs = false;
      string sarg;
      ifilename = "";
      display = "plain";
      rel_type = "purchase";
      number_events=-1;
      K=-1;
      ofilename = "";
    //  Process the input arguments
    for (int i = 1; i < argc;) { // load of the arguments
        sarg = argv[i];
        if (sarg == "-input") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            ifilename = argv[i++];
        } else if (sarg == "-output") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            ofilename = argv[i++];
        }else if (sarg == "-type") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            rel_type = argv[i++];
        }
        else if (sarg == "-display") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            display = argv[i++];
        } else if (sarg == "-K") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            K = atoi(argv[i++]);
        } else if (sarg == "-events") {
            i++;
            if (i >= argc) errorBreak(ERROR_ARGUMENTS, "");
            number_events = atoi(argv[i++]);
        } else if (sarg == "-noargs") {
            i++;
            noargs = true;
        } else
            errorBreak(ERROR_ARGUMENTS, ""); // any other parameters
    }
      if (ifilename == ""||K==-1||number_events==-1)
          errorBreak(ERROR_ARGUMENTS, ""); 
      if (ofilename == "")
        ofilename = ifilename + ".pgrk";
      return noargs;
}

void readInputParameters(string & ifilename, string & display,string & rel_type, int &K, int & number_events,string &ofilename ){
string cad;
      ifilename = "";
      display = "plain";
      rel_type = "purchase";
      number_events=-1;
      K=-1;
      ofilename = "";
    cout << endl << "Please type the input file name: ";
    cin >> ifilename;
    cout << endl << "Please type the number of recommendations: ";
    cin >> K;
    cout << endl << "Please type the number of events to read: ";
    cin >> number_events;
    cout << endl << "Please type Type for the relationship (intro set default value to " << rel_type << "):";
    cin >> cad;
    if (cad != "") rel_type = cad;
    cout << endl << "Please type display (intro set default value to " << display << "):";
    cin >> cad;
    if (cad != "") display = cad;
    ofilename = ifilename + ".pgrk" ;
    cout << endl << "Please type ofilename (intro set default value to " << ofilename << "):";
    cin >> cad;
    if (cad != "") ofilename = cad;    
}

void shopping5(int argc, char **argv){
    EventSet evs;
    string ifilename;
    string ofilename;
    string sarg;
    string display;
    string rel_type;
    int number_events;
    int K;
    bool noargs = false;
    // 1.- Process the input arguments
    noargs = processInputParameters(argc, argv,  ifilename,  display, rel_type, K,  number_events,ofilename );
    if (noargs)
        readInputParameters(ifilename,  display, rel_type, K,  number_events,ofilename );
    
    //2.-Read the file and select biuld the index in order to select the
    // proper brands which are suitable for recommendation, i.e, those which 
    // will be included as row/column in the adjacency matrix.
    ifstream ifile;
    ifile.open(ifilename); // ifilename != ""  already check 
    if (!ifile) {
        errorBreak(ERROR_OPEN, ifilename);
    }
    cout << endl << "Reading from " << ifilename << endl;
    evs.read(ifile, number_events);
    string uniqueBrands[MAXBRANDS];
    int n_values = 0;
    Index idxSesion;
    idxSesion.build(evs, 2);
    cout << "Number of events with non null session in index: " << idxSesion.size() << endl;
    
    idxSesion = rawFilterIndex(evs, idxSesion, "Type", rel_type);
    cout << "After filtering sessions (type == "<< rel_type<<"): " << idxSesion.size() << " sessions" << endl;
            
    findUniqueBrandsInIndex(evs, idxSesion, uniqueBrands, n_values); // also filter unknwon brands
    cout << "There are " << n_values << " unique brands in the filtered sessions" << endl;

    //3. Create the adjacency matrix 

    Matrix m_ady(uniqueBrands, n_values, uniqueBrands, n_values);
    createAdjacencyMatrix(evs, idxSesion, m_ady);
    cout << "Adjacency Matrix " << endl;
    cout << m_ady << endl;

    //4. Compute the recommendations and display the results using -display input parameter (default plain)
    Matrix pagerank;
    pagerank = computePageRank(m_ady, 200, 0.0001);
    // Save the computed values in the output file
    pagerank.save(ofilename);
    // Select the best recommendations
    DataVector recommendations;
    recommendations = selectKbest(pagerank, K);

    if (display == "Fancy") {
        cout << endl << recommendations.showFancyReportH(4) << endl;
    } else if (display == "Framed") {
        cout << endl << recommendations.showFramedReportH(4) << endl;
    } else {
        cout << endl << recommendations.showPlainReportH(4) << endl;
    }

    // 5.- Report everything to the tests suite using REPORT_DATA2
    REPORT_DATA2(recommendations, "Top recommendations");
}