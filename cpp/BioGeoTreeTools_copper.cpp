/*
 * PhyloTree.cpp
 *
 *  Created on: Aug 15, 2009
 *      Author: Stephen A. Smith
 */

#include "BioGeoTreeTools_copper.h"
#include "RateMatrixUtils.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
using namespace std;


#include "tree.h"
#include "tree_reader.h"
#include "node.h"
#include "vector_node_object.h"
#include "string_node_object.h"

Tree * BioGeoTreeTools_copper::getTreeFromString(string treestring){
	TreeReader tr;
	Tree * tree = &tr.readTree(treestring);
	return tree;
}

vector<Node *> BioGeoTreeTools_copper::getAncestors(Tree & tree, Node & nodeId){
	vector<Node *> nodes;
	Node * current = &nodeId;
	while(current->hasParent()){
		current = current->getParent();
		nodes.push_back(current);
	}
	return nodes;
}

void BioGeoTreeTools_copper::summarizeSplits(Node * node,map<vector<int>,vector<AncSplit> > & ans,map<int,string> &areanamemaprev, RateModel * rm){
	double best = 0;
	double sum = 0;
	int areasize = (*ans.begin()).first.size();
	map<int, vector<int> > * distmap = rm->get_int_dists_map(); 
	vector<int> bestldist;
	vector<int> bestrdist;
	map<vector<int>,vector<AncSplit> >::iterator it;
	map<double,string > printstring;
	for(it=ans.begin();it!=ans.end();it++){
		vector<int> dis = (*it).first;
		vector<AncSplit> tans = (*it).second;
		for (unsigned int i=0;i<tans.size();i++){
			if (tans[i].getLikelihood() > best){
				best = tans[i].getLikelihood();
				bestldist = (*distmap)[tans[i].ldescdistint];//tans[i].getLDescDist();
				bestrdist = (*distmap)[tans[i].rdescdistint];//tans[i].getRDescDist();
			}
			//cout << tans[i].getLikelihood() << endl;
			sum += tans[i].getLikelihood();
		}
	}
	for(it=ans.begin();it!=ans.end();it++){
		vector<int> dis = (*it).first;
		vector<AncSplit> tans = (*it).second;
		for (unsigned int i=0;i<tans.size();i++){
			if ((log(best)-log(tans[i].getLikelihood()) ) < 2){
				string tdisstring ="";
				int  count1 = 0;
				for(int m=0;m<areasize;
					//tans[i].getLDescDist().size();
					m++){
					//if(tans[i].getLDescDist()[m] == 1){
					if((*distmap)[tans[i].ldescdistint][m] == 1){
						tdisstring += areanamemaprev[m];
						count1 += 1;
						if(count1 < calculate_vector_int_sum(&(*distmap)[tans[i].ldescdistint])){
							tdisstring += "_";
						}
					}

				}tdisstring += "|";
				count1 = 0;
				for(int m=0;m<areasize;
					//tans[i].getRDescDist().size();
					m++){
					if((*distmap)[tans[i].rdescdistint][m] == 1){
						tdisstring += areanamemaprev[m];
						count1 += 1;
						if(count1 < calculate_vector_int_sum(&(*distmap)[tans[i].rdescdistint])){
							tdisstring += "_";
						}
					}
				}
				printstring[-tans[i].getLikelihood()] = tdisstring;
			}
		}
	}
 	map<double,string >::iterator pit;
	for(pit=printstring.begin();pit != printstring.end();pit++){
		cout << "\t" << (*pit).second << "\t" << (-(*pit).first)/sum << "\t(" << -log(-(*pit).first) << ")"<< endl;
	}
	StringNodeObject disstring ="";
	int  count = 0;
	for(unsigned int m=0;m<bestldist.size();m++){
		if(bestldist[m] == 1){
			disstring += areanamemaprev[m];
			count += 1;
			//if(count < calculate_vector_int_sum(&bestldist))
			if(count < accumulate(bestldist.begin(),bestldist.end(),0))
				disstring += "_";
		}
	}disstring += "|";
	count = 0;
	for(unsigned int m=0;m<bestrdist.size();m++){
		if(bestrdist[m] == 1){
			disstring += areanamemaprev[m];
			count += 1;
			//if(count < calculate_vector_int_sum(&bestrdist))
			if(count < accumulate(bestrdist.begin(),bestrdist.end(),0))
				disstring += "_";
		}
	}
	string spl = "split";
	node->assocObject(spl,disstring);
	//cout << -log(best) << " "<< best/sum << endl;
}

void BioGeoTreeTools_copper::summarizeAncState(Node * node,vector<double> & ans,map<int,string> &areanamemaprev, RateModel * rm){
	double best = 0;
	double sum = 0;
	int areasize = rm->get_num_areas();
	map<int, vector<int> > * distmap = rm->get_int_dists_map(); 
	vector<int> bestancdist;
	map<double,string > printstring;
	for(unsigned int i=0;i<ans.size();i++){
		if (ans[i] > best){
			best = ans[i];
			bestancdist = (*distmap)[i];
		}
		sum += ans[i];
	}
	for(unsigned int i=0;i<ans.size();i++){
		if ((log(best)-log(ans[i]) ) < 2){
			string tdisstring ="";
			int  count1 = 0;
			for(int m=0;m<areasize;m++){
				if((*distmap)[i][m] == 1){
					tdisstring += areanamemaprev[m];
					count1 += 1;
					if(count1 < calculate_vector_int_sum(&(*distmap)[i])){
						tdisstring += "_";
					}
				}
			}
			printstring[-ans[i]] = tdisstring;
		}
	}
 	map<double,string >::iterator pit;
	for(pit=printstring.begin();pit != printstring.end();pit++){
		cout << "\t" << (*pit).second << "\t" << (-(*pit).first)/sum << "\t(" << -log(-(*pit).first) << ")"<< endl;
	}
	StringNodeObject disstring ="";
	int  count = 0;
	for(unsigned int m=0;m<bestancdist.size();m++){
		if(bestancdist[m] == 1){
			disstring += areanamemaprev[m];
			count += 1;
			//if(count < calculate_vector_int_sum(&bestldist))
			if(count < accumulate(bestancdist.begin(),bestancdist.end(),0))
				disstring += "_";
		}
	}
	string spl = "state";
	node->assocObject(spl,disstring);
	//cout << -log(best) << " "<< best/sum << endl;	
}

