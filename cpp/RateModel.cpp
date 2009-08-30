/*
 * RateMatrix.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: smitty
 */

#define VERBOSE false

#include "RateModel.h"
#include "RateMatrixUtils.h"
#include "Utils.h"
//#include "AncSplit.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
using namespace std;

RateModel::RateModel(int na, bool ge, vector<double> pers, bool sp){
	nareas = na;
	globalext = ge;
	periods = pers;
	sparse = sp;
}

void RateModel::setup_dists(){
	map< int, vector<int> > a = iterate_all_bv(nareas);
	if (globalext){
		vector<int> empt;
		for (unsigned int i=0;i<a[0].size();i++){
			empt.push_back(0);
		}
		dists.push_back(empt);
	}
	map<int, vector<int> >::iterator pos;
	for (pos = a.begin(); pos != a.end(); ++pos){
		int f = pos->first;
		dists.push_back(a[f]);
	}
	/*
	 * precalculate the iterdists
	 */
	iter_all_dist_splits();
	if (VERBOSE){
		cout << "dists" <<endl;
		for (unsigned int j=0; j< dists.size(); j++){
			cout << j << " ";
			for (unsigned int i=0;i<dists[j].size();i++){
				cout << dists[j][i];
			}
			cout << endl;
		}
	}
}

/*
 * need to make a generator function for setting distributions
 */
void RateModel::setup_dists(vector<vector<int> > indists, bool include){
	if(include == true){
		dists = indists;
//		if(calculate_vector_int_sum(&dists[0]) > 0){
		if(accumulate(dists[0].begin(),dists[0].end(),0) > 0){
			vector<int> empt;
			for (unsigned int i=0;i<dists[0].size();i++){
				empt.push_back(0);
			}
			dists.push_back(empt);
		}
	}else{//exclude is sent
		vector<int> empt;
		for (int i=0;i<nareas;i++){
			empt.push_back(0);
		}
		dists.push_back(empt);

		map< int, vector<int> > a = iterate_all_bv(nareas);
		map<int, vector<int> >::iterator pos;
		for (pos = a.begin(); pos != a.end(); ++pos){
			int f = pos->first;
			bool inh = false;
			for(unsigned int j=0;j<indists.size();j++){
				//if(is_vector_int_equal_to_vector_int(indists[j],a[f])){
				if(indists[j]==a[f]){
					inh = true;
				}
			}
			if(inh == false)
				dists.push_back(a[f]);
		}
	}
	/*
	 * precalculate the iterdists
	 */
	iter_all_dist_splits();
	if (VERBOSE){
		cout << "dists" <<endl;
		for (unsigned int j=0; j< dists.size(); j++){
			cout << j << " ";
			for (unsigned int i=0;i<dists[j].size();i++){
				cout << dists[j][i];
			}
			cout << endl;
		}
	}
}


/*
void RateModel::remove_dist(vector<int> dist);*/

/*
 * just give Dmask a bunch of ones
 * specify particular ones in the Dmask_cell
 */
void RateModel::setup_Dmask(){
	vector<double> cols(nareas, 1);
	vector< vector<double> > rows(nareas, cols);
	Dmask = vector< vector< vector<double> > > (periods.size(), rows);
}

void RateModel::set_Dmask_cell(int period, int area, int area2, double prob, bool sym){
	Dmask[period][area][area2] = prob;
	if (sym)
		Dmask[period][area2][area] = prob;
}

void RateModel::setup_D(double d){
	vector<double> cols(nareas, 1*d);
	vector< vector<double> > rows(nareas, cols);
	D = vector< vector< vector<double> > > (periods.size(), rows);
	for (unsigned int i=0;i<D.size();i++){
		for (unsigned int j=0;j<D[i].size();j++){
			D[i][j][j] = 0.0;
			for (unsigned int k=0;k<D[i][j].size();k++){
				D[i][j][k] = D[i][j][k] * Dmask[i][j][k];
			}
		}
	}
	if (VERBOSE){
		cout << "D" <<endl;
		for (unsigned int i=0;i<D.size();i++){
			for (unsigned int j=0;j<D[i].size();j++){
				for (unsigned int k=0;k<D[i][j].size();k++){
					cout << D[i][j][k] << " ";
				}
				cout << endl;
			}
			cout << endl;
		}
	}
}

void RateModel::setup_E(double e){
	vector<double> cols(nareas, 1*e);
	E = vector<vector<double> > (periods.size(), cols);
}

void RateModel::set_Qdiag(int period){
	for (unsigned int i=0;i<dists.size();i++){
		double sum =(calculate_vector_double_sum(Q[period][i]) - Q[period][i][i]) * -1.0;
		Q[period][i][i] = sum;
	}
}

void RateModel::setup_Q(){
	vector<double> cols(dists.size(), 0);
	vector< vector<double> > rows(dists.size(), cols);
	Q = vector< vector< vector<double> > > (periods.size(), rows);
	for(unsigned int p=0; p < Q.size(); p++){//periods
		for(unsigned int i=0;i<dists.size();i++){//dists
			//int s1 = calculate_vector_int_sum(&dists[i]);
			int s1 = accumulate(dists[i].begin(),dists[i].end(),0);
			if(s1 > 0){
				for(unsigned int j=0;j<dists.size();j++){//dists
					int sxor = calculate_vector_int_sum_xor(dists[i], dists[j]);
					if (sxor == 1){
						//int s2 = calculate_vector_int_sum(&dists[j]);
						int s2 = accumulate(dists[j].begin(),dists[j].end(),0);
						int dest = locate_vector_int_single_xor(dists[i],dists[j]);
						double rate = 0.0;
						if (s1 < s2){
							for (unsigned int src=0;src<dists[i].size();src++){
								if(dists[i][src] != 0){
									rate += D[p][src][dest] ;//* Dmask[p][src][dest];
								}
							}
						}else{
							rate = E[p][dest];
						}
						Q[p][i][j] = rate;
					}
				}
			}
		}
		set_Qdiag(p);
	}
	/*
	 * sparse needs to be transposed for matrix exponential calculation
	 */
	if(sparse == true){
		vector<double> cols(dists.size(), 0);
		vector< vector<double> > rows(dists.size(), cols);
		QT = vector< vector< vector<double> > > (periods.size(), rows);
		for(unsigned int p=0; p < QT.size(); p++){//periods
			for(unsigned int i=0;i<dists.size();i++){//dists
				for(unsigned int j=0;j<dists.size();j++){//dists
					QT[p][j][i] = Q[p][i][j];
				}
			}
		}
		//setting up the coo numbs
		nzs = vector<int>(Q.size(),0);
		for(unsigned int p=0; p < Q.size(); p++){//periods
			nzs[p] = get_size_for_coo(Q[p],1);
		}
		//setup matrix
		ia_s.clear();
		ja_s.clear();
		a_s.clear();
		for(unsigned int p=0; p < Q.size(); p++){//periods
			vector<int> ia = vector<int>(nzs[p]);
			vector<int> ja = vector<int>(nzs[p]);
			vector<double> a = vector<double>(nzs[p]);
			convert_matrix_to_coo_for_fortran_vector(QT[p],ia,ja,a);//need to multiply these all these by t
			ia_s.push_back(ia);
			ja_s.push_back(ja);
			a_s.push_back(a);
		}
	}
	if(VERBOSE){
		cout << "Q" <<endl;
		for (unsigned int i=0;i<Q.size();i++){
			for (unsigned int j=0;j<Q[i].size();j++){
				for (unsigned int k=0;k<Q[i][j].size();k++){
					cout << Q[i][j][k] << " ";
				}
				cout << endl;
			}
			cout << endl;
		}
	}
}

vector<vector<double > > RateModel::setup_P(int period, double t){
	/*
	return P, the matrix of dist-to-dist transition probabilities,
	from the model's rate matrix (Q) over a time duration (t)
	*/
	vector<vector<double> > p = QMatrixToPmatrix(Q[period], t);

	//filter out impossible dists
	//vector<vector<int> > dis = enumerate_dists();
	for (unsigned int i=0;i<dists.size();i++){
		//if (calculate_vector_int_sum(&dists[i]) > 0){
		if(accumulate(dists[i].begin(),dists[i].end(),0) > 0){
			for(unsigned int j=0;j<dists[i].size();j++){
				if(dists[i][j]==1){//present
					double sum1 =calculate_vector_double_sum(Dmask[period][j]);
					double sum2 = 0.0;
					for(unsigned int k=0;k<Dmask[period].size();k++){
						sum2 += Dmask[period][k][j];
					}
					if(sum1+sum2 == 0){
						for(unsigned int k=0;k<p[period].size();k++){
							p[period][k] = p[period][k]*0.0;
						}
						break;
					}
				}
			}
		}
	}
	if(VERBOSE){
		cout << "p " << period << " "<< t << endl;
		for (unsigned int i=0;i<p.size();i++){
			for (unsigned int j=0;j<p[i].size();j++){
				cout << p[i][j] << " ";
			}
			cout << endl;
		}
	}
	return p;
}


extern"C" {
	void wrapalldmexpv_(int * n,int* m,double * t,double* v,double * w,double* tol,
		double* anorm,double* wsp,int * lwsp,int* iwsp,int *liwsp, int * itrace,int *iflag,
		int *ia, int *ja, double *a, int *nz, double * res);
	void wrapsingledmexpv_(int * n,int* m,double * t,double* v,double * w,double* tol,
			double* anorm,double* wsp,int * lwsp,int* iwsp,int *liwsp, int * itrace,int *iflag,
			int *ia, int *ja, double *a, int *nz, double * res);
	void wrapdgpadm_(int * ideg,int * m,double * t,double * H,int * ldh,
			double * wsp,int * lwsp,int * ipiv,int * iexph,int *ns,int *iflag );
}

/*
 * runs the basic padm fortran expokit full matrix exp
 */
vector<vector<double > > RateModel::setup_fortran_P(int period, double t){
	/*
	return P, the matrix of dist-to-dist transition probabilities,
	from the model's rate matrix (Q) over a time duration (t)
	*/
	int ideg = 6;
	int m = Q[period].size();
	int ldh = m;
	double tol = 1;
	int iflag = 0;
	int lwsp = 4*m*m+6+1;
	double * wsp = new double[lwsp];
	int * ipiv = new int[m];
	int iexph = 0;
	int ns = 0;
	double * H = new double [m*m];
	convert_matrix_to_single_row_for_fortran(Q[period],t,H);
	wrapdgpadm_(&ideg,&m,&tol,H,&ldh,wsp,&lwsp,ipiv,&iexph,&ns,&iflag);
	vector<vector<double> > p (Q[period].size(), vector<double>(Q[period].size()));
	for(int i=0;i<m;i++){
		for(int j=0;j<m;j++){
			p[i][j] = wsp[iexph+(j-1)*m+(i-1)+m];
		}
	}
	delete [] wsp;
	delete [] ipiv;
	delete [] H;
	for(unsigned int i=0; i<p.size(); i++){
		double sum = 0.0;
		for (unsigned int j=0; j<p[i].size(); j++){
			sum += p[i][j];
		}
		for (unsigned int j=0; j<p[i].size(); j++){
			p[i][j] = (p[i][j]/sum);
		}
	}


	//filter out impossible dists
	//vector<vector<int> > dis = enumerate_dists();
	/*
	for (unsigned int i=0;i<dists.size();i++){
		//if (calculate_vector_int_sum(&dists[i]) > 0){
		if(accumulate(dists[i].begin(),dists[i].end(),0) > 0){
			for(unsigned int j=0;j<dists[i].size();j++){
				if(dists[i][j]==1){//present
					double sum1 =calculate_vector_double_sum(Dmask[period][j]);
					double sum2 = 0.0;
					for(unsigned int k=0;k<Dmask[period].size();k++){
						sum2 += Dmask[period][k][j];
					}
					if(sum1+sum2 == 0){
						for(unsigned int k=0;k<p[period].size();k++){
							p[period][k] = p[period][k]*0.0;
						}
						break;
					}
				}
			}
		}
	}*/
	if(VERBOSE){
		cout << "p " << period << " "<< t << endl;
		for (unsigned int i=0;i<p.size();i++){
			for (unsigned int j=0;j<p[i].size();j++){
				cout << p[i][j] << " ";
			}
			cout << endl;
		}
	}
	return p;
}

/*
 * runs the sparse matrix fortran expokit matrix exp
 */
vector<vector<double > > RateModel::setup_sparse_full_P(int period, double t){
	int n = Q[period].size();
	int m = Q[period].size()-1;//tweak
	int nz = get_size_for_coo(Q[period],1);
	int * ia = new int [nz];
	int * ja = new int [nz];
	double * a = new double [nz];
	convert_matrix_to_coo_for_fortran(Q[period],t,ia,ja,a);
	double * v = new double [n];
	for(int i=0;i<n;i++){
		v[i] = 0;
	}v[0]= 1;
	double * w = new double [n];
	int ideg = 6;
	double tol = 1;
	int iflag = 0;
	int lwsp = n*(m+1)+n+pow((m+2),2)+4*pow((m+2),2)+ideg+1;
	double * wsp = new double[lwsp];
	int liwsp = m+2;
	int * iwsp = new int [liwsp];
	double t1 = 1;
	double anorm = 0;
	int itrace = 0;
	double * res = new double [n*n];
	wrapalldmexpv_(&n,&m,&t1,v,w,&tol,&anorm,wsp,&lwsp,iwsp,&liwsp,
			&itrace,&iflag,ia,ja,a,&nz,res);

	vector<vector<double> > p (Q[period].size(), vector<double>(Q[period].size()));
	int count = 0;
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			p[j][i] = res[count];
			count += 1;
		}
	}

	//filter out impossible dists
	//vector<vector<int> > dis = enumerate_dists();
	for (unsigned int i=0;i<dists.size();i++){
		//if (calculate_vector_int_sum(&dists[i]) > 0){
		if(accumulate(dists[i].begin(),dists[i].end(),0) > 0){
			for(unsigned int j=0;j<dists[i].size();j++){
				if(dists[i][j]==1){//present
					double sum1 =calculate_vector_double_sum(Dmask[period][j]);
					double sum2 = 0.0;
					for(unsigned int k=0;k<Dmask[period].size();k++){
						sum2 += Dmask[period][k][j];
					}
					if(sum1+sum2 == 0){
						for(unsigned int k=0;k<p[period].size();k++){
							p[period][k] = p[period][k]*0.0;
						}
						break;
					}
				}
			}
		}
	}
	delete []res;
	delete []iwsp;
	delete []w;
	delete []wsp;
	delete []a;
	delete []ia;
	delete []ja;
	if(VERBOSE){
		cout << "p " << period << " "<< t << endl;
		for (unsigned int i=0;i<p.size();i++){
			for (unsigned int j=0;j<p[i].size();j++){
				cout << p[i][j] << " ";
			}
			cout << endl;
		}
	}
	return p;
}

/*
 * for returning single P columns
 */
vector<double >  RateModel::setup_sparse_single_column_P(int period, double t, int column){
	int n = Q[period].size();
	int m = nareas-1;
	int nz = nzs[period];//get_size_for_coo(Q[period],1);
	int * ia = new int [nz];
	int * ja = new int [nz];
	double * a = new double [nz];
	//convert_matrix_to_coo_for_fortran(QT[period],1,ia,ja,a);
	std::copy(ia_s[period].begin(), ia_s[period].end(), ia);
	std::copy(ja_s[period].begin(), ja_s[period].end(), ja);
	std::copy(a_s[period].begin(), a_s[period].end(), a);
	double * v = new double [n];
	for(int i=0;i<n;i++){
		v[i] = 0;
	}v[column]= 1; // only return the one column we want
	double * w = new double [n];
	int ideg = 6;
	double tol = 1;
	int iflag = 0;
	int lwsp = n*(m+1)+n+pow((m+2),2)+4*pow((m+2),2)+ideg+1;
	double * wsp = new double[lwsp];
	int liwsp = m+2;
	int * iwsp = new int [liwsp];
	double t1 = t;//use to be 1
	double anorm = 0;
	int itrace = 0;
	double * res = new double [n]; // only needs resulting columns
	wrapsingledmexpv_(&n,&m,&t1,v,w,&tol,&anorm,wsp,&lwsp,iwsp,&liwsp,
			&itrace,&iflag,ia,ja,a,&nz,res);

	vector<double> p (Q[period].size());
	int count = 0;
	for(int i=0;i<n;i++){
		p[i] = res[count];
		count += 1;
	}
	delete []v;
	delete []res;
	delete []iwsp;
	delete []w;
	delete []wsp;
	delete []a;
	delete []ia;
	delete []ja;
	if(VERBOSE){
		cout << "p " << period << " "<< t << " " << column <<  endl;
		for (unsigned int i=0;i<p.size();i++){
			cout << p[i] << " ";
		}
		cout << endl;
	}
	return p;
}

vector<vector<vector<int> > > RateModel::iter_dist_splits(vector<int> & dist){
	//assert dist in dists
	vector< vector <vector<int> > > ret;
	vector< vector<int> > left;
	vector< vector<int> > right;
//	if(calculate_vector_int_sum(&dist) == 1){
	if(accumulate(dist.begin(),dist.end(),0) == 1){
		left.push_back(dist);
		right.push_back(dist);
	}
	else{
		for(unsigned int i=0;i<dist.size();i++){
			if (dist[i]==1){
				vector<int> x(dist.size(),0);
				x[i] = 1;
				int cou = count(dists.begin(),dists.end(),x);
				//if (is_vector_int_in_multi_vector_int(x,dists)){
				if(cou > 0){
					left.push_back(x);right.push_back(dist);
					left.push_back(dist);right.push_back(x);
					vector<int> y;
					for(unsigned int j=0;j<dist.size();j++){
						if(dist[j]==x[j]){
							y.push_back(0);
						}else{
							y.push_back(1);
						}
					}
					int cou2 = count(dists.begin(),dists.end(),y);
					//if (is_vector_int_in_multi_vector_int(y,dists)){
					if(cou2 > 0){
						left.push_back(x);right.push_back(y);
						//if(calculate_vector_int_sum(&y)>1){
						if(accumulate(y.begin(),y.end(),0) > 1){
							left.push_back(y);right.push_back(x);
						}
					}
				}
			}
		}
	}
	if(VERBOSE){
		cout << "LEFT" << endl;
		for(unsigned int i = 0; i< left.size() ; i++ ){
			print_vector_int(left[i]);
		}
		cout << "RIGHT" << endl;
		for(unsigned int i = 0; i< right.size() ; i++ ){
			print_vector_int(right[i]);
		}
	}
	ret.push_back(left);
	ret.push_back(right);
	return ret;
}

void RateModel::iter_all_dist_splits(){
	for(unsigned int i=0;i<dists.size();i++){
		iter_dists[dists[i]] = iter_dist_splits(dists[i]);
	}
}


vector<vector<int> > * RateModel::getDists(){
	return &dists;
}

vector<vector<vector<int> > > RateModel::get_iter_dist_splits(vector<int> & dist){
	return iter_dists[dist];
}

/**/


