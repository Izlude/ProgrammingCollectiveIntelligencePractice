#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string.h>

#define USER 1000
#define ITEM 2000
#define DATA_FILE "u.data"

using namespace std;

bool original_sort(pair <int, double> a, pair <int, double> b){
    return a.second > b.second;
}

/* Returns a distance-based similarity score for person1 and person2 */
double simDistance(vector <vector <int> > prefs, int person1, int person2){
    /* Get the list of shared_items */
    int si[ITEM], n = 0;
    memset(si, 0, sizeof(si));

    for(int item = 0; item < prefs[person1].size(); item++){
	if(prefs[person1][item] != -1 && prefs[person2][item] != -1){
	    si[item] = 1;
	    n++;
	}
    }

    /* if they have no ratings in common, return 0 */
    if(n == 0) return 0;

    /* Add up the squares of all the differences */
    long sum_of_squares = 0;
    for(int item = 0; item < prefs[0].size(); item++) if(si[item]){
    	sum_of_squares += pow(prefs[person1][item] - prefs[person2][item], 2);
    }

    return 1.0 / (1.0 + sum_of_squares);
}

/* Returns the Pearson correlation coefficient for person1 and person2 */
double simPearson(vector <vector <int> > prefs, int person1, int person2){
    /* Get the list of shared_items */
    int si[ITEM], n = 0;
    memset(si, 0, sizeof(si));

    for(int item = 0; item < prefs[person1].size(); item++){
	if(prefs[person1][item] != -1 && prefs[person2][item] != -1){
	    si[item] = 1;
	    n++;
	}
    }

    /* if they have no ratings in common, return 0 */
    if(n == 0) return 0;

    long sum1 = 0, sum2 = 0, sum1Sq = 0, sum2Sq = 0, pSum = 0;
    double num, den;
    /* Add up all the preferences, Sum up the squares, Sum up the products */
    for(int item = 0; item < prefs[0].size(); item++) if(si[item]){
    	sum1 += prefs[person1][item];
	sum2 += prefs[person2][item];
	sum1Sq += pow(prefs[person1][item], 2);
	sum2Sq += pow(prefs[person2][item], 2);
	pSum += prefs[person1][item] * prefs[person2][item];
    }

    num = pSum - (1.0 * sum1 * sum2 / n);
    den = sqrt((sum1Sq - 1.0 * pow(sum1, 2)/n) * (sum2Sq - 1.0 * pow(sum2, 2)/n));
    if(den == 0) return 0;

    return num / den;
}

vector < pair <int, double> > topMatches(vector <vector <int> > prefs, 
	int person, int num = 5, 
	double (*similarity)(vector <vector <int> > p, int p1, int p2) = simPearson){
    
    /* Returns the best matches for person from the prefs dictionary.
       Number of results ans similaryty function are optional params. */
    vector < pair <int, double> > scores(prefs.size());
    for(int other = 0; other < prefs.size(); other++) if(person != other){
    	scores[other] = make_pair(other, similarity(prefs, person, other));
    }

    /* Sort the list so the highest scores appear at the top */
    sort(scores.begin(), scores.end(), original_sort);
    scores.erase(scores.begin() + num, scores.end());

    return scores;
}

/* Gets recommendations for a person by using a wrighted average
   of every other user's rankings */
vector < pair <int, double> > getRecommendations(vector <vector <int > > prefs,
	int person, int num = 10,
	double (*similarity)(vector <vector <int> > p, int p1, int p2) = simPearson){
    double totals[prefs[0].size()];
    double simSums[prefs[0].size()];

    memset(totals, 0, sizeof(totals));
    memset(simSums, 0, sizeof(simSums));
    
    for(int other = 0; other < prefs.size(); other++){
	/* don't compare myself */
	if(person == other) continue;
	double sim = similarity(prefs, person, other);
	/* ignore scores of zero or lower */
	if(sim <= 0) continue;

	for(int item = 0; item < prefs[0].size(); item++){
	    /* only score movies I haven't seen yes */
	    if(prefs[person][item] <= 0 && prefs[other][item] > 0){
	    	/* Similarity * Score */
		totals[item] += prefs[other][item] * sim;
		/* Sum of similarities */
		simSums[item] += sim;
	    }
	}
    }

    /* Create the normalized list */
    vector < pair <int, double> > rankings(prefs[0].size());
    for(int item = 0; item < prefs[0].size(); item++) if(simSums[item]){
	rankings[item] = make_pair(item, totals[item] / simSums[item]);
    }
    sort(rankings.begin(), rankings.end(), original_sort);
    rankings.erase(rankings.begin() + num, rankings.end());
    return rankings;
}

vector <vector <int> > transformPerfs(vector <vector <int> > prefs){
    vector <vector <int> > result(prefs[0].size(), vector <int>(prefs.size()));
    
    for(int person = 0; person < prefs.size(); person++)
	for(int item = 0; item < prefs[0].size(); item++)
	    result[item][person] = prefs[person][item];
    
    return result;
}

vector < vector < pair <int, double> > >
calculateSimilarItems(vector <vector <int> > prefs,
	int num = 10){
    /* Create a dictionary of items showing which other items they
       are most similar to */
    vector < vector < pair <int, double> > > result(prefs[0].size());

    /* Invert the preference matrix to be item-centric */
    vector <vector <int> > itemPrefs = transformPerfs(prefs);
    int cnt = 0;
    for(int item = 0; item < itemPrefs.size(); item++){
    	/* Status updates for large datasets */
	cnt++;
	if(cnt % 100 == 0) cout << cnt / itemPrefs.size() << "%" << endl;
	/* Find the most similar items to this one */
	vector < pair <int, double> > scores 
	    = topMatches(itemPrefs, item, num, simDistance);
	result[item] = scores;
    }
    return result;
}

#if 0
void getRecommendedItems(vector <vector <int> > prefs,
	vector < vector < pair <int, double > > > itemMatch, int user){
    /* Loop over items rated by user */
    for(int item = 0; item < 10; item++){

    }
}
#endif

int main(){
    vector <vector <int> > data(USER, vector <int>(ITEM, -1));

    // set data
    ifstream ifs(DATA_FILE);
    string buf;
    
    while(ifs && getline(ifs, buf)) {
	int user_id, item_id, rating;
	long timestamp;

	sscanf(buf.c_str(), "%d %d %d %ld",
		&user_id, &item_id, &rating, &timestamp);
	data[user_id][item_id] = rating;
    }

    // cal distance
    cout << "--- cal distance ---" << endl;
    cout << simDistance(data, 12, 20) << endl;
    cout << simPearson(data, 12, 20) << endl;

    // topMatches
    cout << "--- top Matches ---" << endl;
    vector < pair <int, double> > s = topMatches(data, 12, 15);
    for(int num = 0; num < s.size(); num++)
	cout << s[num].first << " " << s[num].second << endl; 
    
    // getRecommendations
    cout << "--- get Recommendations ---" << endl;
    vector < pair <int, double> > r = getRecommendations(data, 12, 15);
    for(int num = 0; num < r.size(); num++)
	cout << r[num].first << " " << r[num].second << endl; 


    // transformPerfs -> topMatches
    cout << "--- transform Perfs -> top Matches ---" << endl;
    vector < vector <int> > result = transformPerfs(data);
    vector < pair <int, double> > ss = topMatches(result, 10, 15);
    for(int num = 0; num < ss.size(); num++)
	cout << ss[num].first << " " << ss[num].second << endl; 

    // transformPerfs -> getRecommendations
    cout << "--- transform Perfs -> get Recommendations ---" << endl;
    vector < pair <int, double> > rr = getRecommendations(result, 12, 15);
    for(int num = 0; num < rr.size(); num++)
	cout << rr[num].first << " " << rr[num].second << endl; 

    // calculateSimilarItems
    cout << "--- calculate SimilarItems ---" << endl;
    vector < vector < pair <int, double> > > rrr
	= calculateSimilarItems(data);
    for(int num1 = 0; num1 < rrr.size(); num1++){
	for(int num2 = 0; num2 < rrr[num1].size(); num2++)
	    cout << rrr[num1][num2].first << " "
		<< rrr[num1][num2].second << endl;
	cout <<endl;
    }

    return 0;
}
