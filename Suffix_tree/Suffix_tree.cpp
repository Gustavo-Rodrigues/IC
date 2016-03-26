//
// STREE2006.CPP - Suffix tree creation
//
// Mark Nelson, updated December, 2006
//
// This code has been tested with Borland C++ and
// Microsoft Visual C++.
//
// This program asks you for a line of input, then
// creates the suffix tree corresponding to the given
// text. Additional code is provided to validate the
// resulting tree after creation.
//
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <string>
#include <string.h>
#include <vector>
#include <stack>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>
#include <alloca.h>
#include <cstring>
#include <math.h>

using namespace std;
using std::cout;
using std::cin;
using std::cerr;
using std::setw;
using std::flush;
using std::endl;
using std::vector;
using std::stack;


//
// When a new tree is added to the table, we step
// through all the currently defined suffixes from
// the active point to the end point.  This structure
// defines a Suffix by its final character.
// In the canonical representation, we define that last
// character by starting at a node in the tree, and
// following a string of characters, represented by
// first_char_index and last_char_index.  The two indices
// point into the input string.  Note that if a suffix
// ends at a node, there are no additional characters
// needed to characterize its last character position.
// When this is the case, we say the node is Explicit,
// and set first_char_index > last_char_index to flag
// that.
//

class Suffix {
    public :
        int origin_node;
        int first_char_index;
        int last_char_index;
        Suffix( int node, int start, int stop )
            : origin_node( node ),
              first_char_index( start ),
              last_char_index( stop ){};
        int Explicit(){ return first_char_index > last_char_index; }
        int Implicit(){ return last_char_index >= first_char_index; }
        void Canonize();
};

//
// The suffix tree is made up of edges connecting nodes.
// Each edge represents a string of characters starting
// at first_char_index and ending at last_char_index.
// Edges can be inserted and removed from a hash table,
// based on the Hash() function defined here.  The hash
// table indicates an unused slot by setting the
// start_node value to -1.
//

class Edge {
    /*
        ADDED:
        - occur_vec vector
        - length_or_position
    */
    public :
        int first_char_index;
        int last_char_index;
        int end_node;
        int start_node;
        //if it is a leaf then it's a position otherwise it's the accumulated length
        int length_or_position;

        //occurence vector
        std::vector<int> occur_vec;
        void Insert();
        void Remove();

        Edge();
        Edge( int init_first_char_index,
              int init_last_char_index,
              int parent_node );

        int SplitEdge( Suffix &s );
        static Edge Find( int node, int c );
        static int Hash( int node, int c );


};

//
//  The only information contained in a node is the
//  suffix link. Each suffix in the tree that ends
//  at a particular node can find the next smaller suffix
//  by following the suffix_node link to a new node.  Nodes
//  are stored in a simple array.
//
class Node {
    public :
        int suffix_node;
        Node() { suffix_node = -1; }
        static int Count;
};

//
// The maximum input string length this program
// will handle is defined here.  A suffix tree
// can have as many as 2N edges/nodes.  The edges
// are stored in a hash table, whose size is also
// defined here.
//

/*
    ADDED:
        - class Period
            - confidence
            - period
*/
class Period{
    public :
        string sequence;
        double confidence;
        int period;
};

const int MAX_LENGTH = 1000;
const int HASH_TABLE_SIZE = 2179;  //A prime roughly 10% larger

//
// This is the hash table where all the currently
// defined edges are stored.  You can dump out
// all the currently defined edges by iterating
// through the table and finding edges whose start_node
// is not -1.
//

Edge Edges[ HASH_TABLE_SIZE ];

//
// The array of defined nodes.  The count is 1 at the
// start because the initial tree has the root node
// defined, with no children.
//

int Node::Count = 1;
Node Nodes[ MAX_LENGTH * 2 ];

//
// The input buffer and character count.  Please note that N
// is the length of the input string -1, which means it
// denotes the maximum index in the input buffer.
//

char T[ MAX_LENGTH ];
int N;

//
// Necessary forward references
//
void validate();
int walk_tree( int start_node, int last_char_so_far );


//
// The default ctor for Edge just sets start_node
// to the invalid value.  This is done to guarantee
// that the hash table is initially filled with unused
// edges.
//

Edge::Edge()
{
    start_node = -1;
}

//
// I create new edges in the program while walking up
// the set of suffixes from the active point to the
// endpoint.  Each time I create a new edge, I also
// add a new node for its end point.  The node entry
// is already present in the Nodes[] array, and its
// suffix node is set to -1 by the default Node() ctor,
// so I don't have to do anything with it at this point.
//

Edge::Edge( int init_first, int init_last, int parent_node )
{
    first_char_index = init_first;
    last_char_index = init_last;
    start_node = parent_node;
    end_node = Node::Count++;
}

//
// Edges are inserted into the hash table using this hashing
// function.
//

int Edge::Hash( int node, int c )
{
    return ( ( node << 8 ) + c ) % HASH_TABLE_SIZE;
}

//
// A given edge gets a copy of itself inserted into the table
// with this function.  It uses a linear probe technique, which
// means in the case of a collision, we just step forward through
// the table until we find the first unused slot.
//

void Edge::Insert()
{
    int i = Hash( start_node, T[ first_char_index ] );
    while ( Edges[ i ].start_node != -1 )
        i = ++i % HASH_TABLE_SIZE;
    Edges[ i ] = *this;
}

//
// Removing an edge from the hash table is a little more tricky.
// You have to worry about creating a gap in the table that will
// make it impossible to find other entries that have been inserted
// using a probe.  Working around this means that after setting
// an edge to be unused, we have to walk ahead in the table,
// filling in gaps until all the elements can be found.
//
// Knuth, Sorting and Searching, Algorithm R, p. 527
//

void Edge::Remove()
{
    int i = Hash( start_node, T[ first_char_index ] );
    while ( Edges[ i ].start_node != start_node ||
            Edges[ i ].first_char_index != first_char_index )
        i = ++i % HASH_TABLE_SIZE;
    for ( ; ; ) {
        Edges[ i ].start_node = -1;
        int j = i;
        for ( ; ; ) {
            i = ++i % HASH_TABLE_SIZE;
            if ( Edges[ i ].start_node == -1 )
                return;
            int r = Hash( Edges[ i ].start_node, T[ Edges[ i ].first_char_index ] );
            if ( i >= r && r > j )
                continue;
            if ( r > j && j > i )
                continue;
            if ( j > i && i >= r )
                continue;
            break;
        }
        Edges[ j ] = Edges[ i ];
    }
}

//
// The whole reason for storing edges in a hash table is that it
// makes this function fairly efficient.  When I want to find a
// particular edge leading out of a particular node, I call this
// function.  It locates the edge in the hash table, and returns
// a copy of it.  If the edge isn't found, the edge that is returned
// to the caller will have start_node set to -1, which is the value
// used in the hash table to flag an unused entry.
//

Edge Edge::Find( int node, int c )
{
    int i = Hash( node, c );
    for ( ; ; ) {
        if ( Edges[ i ].start_node == node )
            if ( c == T[ Edges[ i ].first_char_index ] )
                return Edges[ i ];
        if ( Edges[ i ].start_node == -1 )
            return Edges[ i ];
        i = ++i % HASH_TABLE_SIZE;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
vector<Edge> get_edges(){
    vector<Edge> edges;
    for(int u = 0; u<HASH_TABLE_SIZE;u++){
        if(Edges[u].start_node != -1){
            edges.push_back(Edges[u]);
        }
    }
    return edges;
}

vector<Edge> init_ocurrence_vector(){

    //auxiliates to pass the values down
    stack<Edge> stack;
    //begins the rising values
    vector<Edge> leaves;
    //continuation of the rising values
    vector<Edge> fathers;
    //edges in the suffix tree
    vector<Edge> edges;
    //save the inner nodes for period analisys
    vector<Edge> inner_edges;

    //initialize the edge vector with the existing edges
    edges = get_edges();

    //print
    /*
    for (std::vector<Edge>::iterator it = edges.begin() ; it != edges.end(); ++it){
        cout << "Start Node: " << (*it).start_node << " End node: " << (*it).end_node
            << " first char: " << (*it).first_char_index << " last char: " << (*it).last_char_index << "\n";
    }
    cout << endl;
    */

    //initialize the stack
    for(int v = 0; v<edges.size(); v++){
        if(edges[v].start_node == 0){
            edges[v].length_or_position = edges[v].last_char_index - edges[v].first_char_index + 1;
            stack.push(edges[v]);
        }
    }

    //get the leaves of the tree
    for (int i = 0; i < edges.size(); i++){
        int equal = 0;
        for (int j = 0 ; j < edges.size(); j++){
            if(edges[i].end_node == edges[j].start_node && edges[i].end_node != edges[j].end_node){
                equal++;
            }
        }
        if(equal == 0 && edges[i].start_node != 0){
            leaves.push_back(edges[i]);
        }
    }

    //THE FALL OF THE VALUES
    //passing the values down
    //cout << "STACK: \n";
    while(stack.size()>0){
        //get the top of the stack
        Edge e = stack.top();
        //print
        //cout << "Start node: " << e.start_node << " End node: " << e.end_node << " Length or position: "
        //<< e.length_or_position << endl;

        //pop the stack
        stack.pop();
        for(int x = 0; x<edges.size(); x++){
            //if there is a child
            if(e.end_node == edges[x].start_node ){
                //it is necessary to check if it is leaf
                bool is_leaf = false;
                for(int y = 0; y<leaves.size(); y++){
                    if(edges[x].end_node == leaves[y].end_node){
                        is_leaf = true;
                        break;
                    }
                }
                //if it is not a leaf
                if(!is_leaf){
                    edges[x].length_or_position = edges[x].last_char_index - edges[x].first_char_index + 1 +
                     e.length_or_position;
                }else{
                    edges[x].length_or_position = edges[x].first_char_index - e.length_or_position;
                    //update the leaves vector
                    for(int l = 0; l<leaves.size(); l++){
                        if(edges[x].end_node == leaves[l].end_node){
                            leaves[l].length_or_position = edges[x].first_char_index - e.length_or_position;
                        }
                    }
                }
                stack.push(edges[x]);
            }
        }
    }

    //THE FIRST RISE OF THE VALUES
    for(int w = 0; w<leaves.size();w++){
        for(int z = 0; z<edges.size(); z++){
            if(leaves[w].start_node == edges[z].end_node){
                //cout << "length or position:" << leaves[w].length_or_position << endl;
                edges[z].occur_vec.push_back(leaves[w].length_or_position);
                //make sure that there is no repetition
                int rep = 0;
                for(int r = 0;r<fathers.size();r++){
                    if(fathers[r].end_node == edges[z].end_node){
                        fathers[r].occur_vec.push_back(leaves[w].length_or_position);
                        rep++;
                    }
                }
                if(rep == 0){
                    fathers.push_back(edges[z]);
                }
            }
        }
    }
    inner_edges = fathers;

    for(int t = 0;t<fathers.size();t++){
        stack.push(fathers[t]);
    }


    //the temp vector makes sure that we go up in the tree level by level
    // updating the respective fathers properly
    /*
    while(fathers.size()>0){
        vector<Edge> temp;
        for(int a = 0; a<fathers.size(); a++){
            //finds grandpa
            for(int b = 0; b<edges.size(); b++){
                //if found a grandpa
                if(fathers[a].start_node == edges[b].end_node){
                    //pass the values
                    for(int c = 0;c<fathers[a].occur_vec.size();c++){
                        edges[b].occur_vec.push_back(fathers[a].occur_vec[c]);
                    }
                    //constructing the inner edges...
                    int repeated = 0;
                    for(int h = 0;h<inner_edges.size();h++){
                        if(inner_edges[h].end_node == edges[b].end_node){
                            for(int k = 0;k<edges[b].occur_vec.size();k++){
                                inner_edges[h].occur_vec.push_back(edges[b].occur_vec[k]);
                            }
                            repeated++;
                            break;
                        }
                    }
                    if(repeated == 0){
                        inner_edges.push_back(edges[b]);
                    }
                    temp.push_back(edges[b]);
                }
            }
        }
        fathers.clear();
        fathers.swap(temp);
        temp.clear();
    }
    */

    //THE FINAL RISE OF THE VALUES
    //the stack contains fathers
    //might have to uncomment the sort part beacuse of memory issues
    while(stack.size()>0){
        Edge e = stack.top();
        stack.pop();
        for(int b = 0; b<edges.size();b++){
            //if found a parent
            if(e.start_node == edges[b].end_node){
                //pass the values
                for(int c = 0;c<e.occur_vec.size();c++){
                    edges[b].occur_vec.push_back(e.occur_vec[c]);

                }
                sort( edges[b].occur_vec.begin(), edges[b].occur_vec.end() );
                edges[b].occur_vec.erase( unique( edges[b].occur_vec.begin(),
                edges[b].occur_vec.end() ), edges[b].occur_vec.end() );
                //put the parent in the stack
                stack.push(edges[b]);
                //constructing the inner edges...
                int repeated = 0;
                for(int h = 0;h<inner_edges.size();h++){
                    if(inner_edges[h].end_node == edges[b].end_node){
                        for(int k = 0;k<edges[b].occur_vec.size();k++){
                            inner_edges[h].occur_vec.push_back(edges[b].occur_vec[k]);
                        }
                        repeated++;
                        break;
                    }
                    /*
                    sort( inner_edges[h].occur_vec.begin(), inner_edges[h].occur_vec.end() );
                    inner_edges[h].occur_vec.erase( unique( inner_edges[h].occur_vec.begin(),
                    inner_edges[h].occur_vec.end() ), inner_edges[h].occur_vec.end() );
                    */
                }
                if(repeated == 0){
                    inner_edges.push_back(edges[b]);
                }
            }
        }
    }



    //sort unique values and create the difference vector
    for(int l = 0;l<inner_edges.size();l++){
        sort( inner_edges[l].occur_vec.begin(), inner_edges[l].occur_vec.end() );
        inner_edges[l].occur_vec.erase( unique( inner_edges[l].occur_vec.begin(),
        inner_edges[l].occur_vec.end() ), inner_edges[l].occur_vec.end() );
    }

    /*
    cout << "INNER EDGES: \n";
    for (vector<Edge>::iterator it2 = inner_edges.begin() ; it2 != inner_edges.end(); ++it2){
        cout << "Start Node: " << (*it2).start_node << " End node: " << (*it2).end_node << " Occurence vector: ";
        for(int occ = 0; occ<(*it2).occur_vec.size(); occ++){
            cout << " " << (*it2).occur_vec[occ];
        }
        cout << endl;
    }
    cout << endl;
    */
    return inner_edges;
}

void write_file(vector<Period> intel ,string subject, string channel){
    ofstream myfile ("example.txt");
    if (myfile.is_open()){
        myfile << "This is a line.\n";
        myfile << "This is another line.\n";
        myfile.close();
    }
    else cout << "Unable to open file";
}


void period_detection(vector<Edge> inner_edges, string subject, string channel){
    vector<Period> intel;
    Period period;


    //for each occurence vector of size k for pattern X repeat
    for(int a=0;a<inner_edges.size();a++){
        for(int j=0;j<inner_edges[a].occur_vec.size();j++){
            //cout << "Start node:" << inner_edges[a].start_node << " End node" << inner_edges[a].end_node
            //<< " "<<inner_edges[a].occur_vec[j]
            //<<endl;
            int count = 0;
            int p = inner_edges[a].occur_vec[j+1] - inner_edges[a].occur_vec[j];
            int stPos = inner_edges[a].occur_vec[0];
            int endPos = inner_edges[a].occur_vec[inner_edges[a].occur_vec.size()-1];
            int cmp = inner_edges[a].occur_vec[j];
            for(int i=j;i<inner_edges[a].occur_vec.size();i++){
                if(cmp%p == inner_edges[a].occur_vec[i]%p){
                    count++;
                }
            }
            double conf = (double) count/floor((strlen(T)-cmp+1)/p);
            if(conf >= 0.9){
                period.confidence = conf;
                period.period = p;
                intel.push_back(period);
            }
            /*
            cout.precision(3);
            cout << "Start node:" << inner_edges[a].start_node << " End node" << inner_edges[a].end_node <<
            " Confidence: " << conf << " count: " << count << " Perfect rep: " << ((strlen(T)-cmp+1)/p) <<
            " period: " << p <<endl;
            */
        }
    }
    //write_file(intel, subject, channel);
}
//creates a substring from the original string based on the start node
string get_substring(int end_node){
    vector<Edge> edges;
    stack<string> stack;
    //cout << "end_node: " << end_node << endl;

    edges = get_edges();
    for(int i = 0; i< edges.size(); i++){
        if(edges[i].end_node == end_node){
            string temp = "";
            for(int j = edges[i].first_char_index; j<= edges[i].last_char_index; j++){
                temp = temp + T[j];
            }
            stack.push(temp);
            if(edges[i].start_node != 0){
                end_node = edges[i].start_node;
                i = 0;
            }
        }
    }
    string substring = "";
    while(stack.size()>0){
        substring = substring + stack.top();
        stack.pop();
    }
    return substring;
}

vector<Period> period_detection_with_tolerance(vector<Edge> inner_edges, int tolerance){
    /*TODO:
        - do the statistics
        - construct the string
        - write the file
    */

    vector<Period> intel;
    Period period;

    for(int a=0;a<inner_edges.size();a++){
        for(int j=0;j<inner_edges[a].occur_vec.size()-1;j++){
            int count = 0;
            //int sumPer = 0;
            int p = inner_edges[a].occur_vec[j+1] - inner_edges[a].occur_vec[j];
            int stPos = inner_edges[a].occur_vec[0];
            int endPos = inner_edges[a].occur_vec[inner_edges[a].occur_vec.size()-1];
            int cmp = inner_edges[a].occur_vec[j];
            int currStPos = cmp;
            int preOccur = cmp+p;
            for(int i=j;i<inner_edges[a].occur_vec.size();i++){
                double A = (double) inner_edges[a].occur_vec[i] - currStPos;
                double B = (double) round(A/p);
                cout.precision(3);
                //cout << "shiet: " << inner_edges[a].occur_vec[i] << " B: " << B << endl;

                int C = A - (p*B);
                if( ((C>=(-1)*tolerance) && C<=tolerance) && (round((preOccur - currStPos)/p) != B) ){
                    currStPos = inner_edges[a].occur_vec[i];
                    preOccur = inner_edges[a].occur_vec[i];
                    //sumPer = sumPer + p + C;
                    count++;
                }
                //cout << "count: " << count << endl;
            }
            //double meanP = (double) sumPer-(p/(count-1));
            double conf = (double) count/(floor((strlen(T)-stPos+1)/p));
            if(conf >= 0.9){
                period.sequence = get_substring(inner_edges[a].end_node);
                period.confidence = conf;
                period.period = p;
                intel.push_back(period);
            }

            /*
            cout.precision(3);
            cout << "Start node:" << inner_edges[a].start_node << " End node" << inner_edges[a].end_node <<
            " Confidence: " << conf << " count: " << count << " Perfect rep: " << floor((strlen(T)-stPos+1)/p) <<
            " period: " << p <<endl;
            */

            /*
            cout << "INNER EDGES: \n";
            cout << "Start Node: " << inner_edges[a].start_node << " End node: " << inner_edges[a].end_node << " Occurence vector: ";
            for(int occ = 0; occ<inner_edges[a].occur_vec.size(); occ++){
                cout << " " << inner_edges[a].occur_vec[occ];
            }
            cout << endl;
            */
        }
    }
    /*
    cout << "Periods: \n";
    for (vector<Period>::iterator it2 = intel.begin() ; it2 != intel.end(); ++it2){
        cout << "Sequence: " << (*it2).sequence << " Confidence:" << (*it2).confidence <<
        " Period: " << (*it2).period << endl;
    }
    cout << endl;
    */
    //write_file(intel, subject, channel);
    return intel;
}

void period_detection_with_tolerance_test(vector<Edge> inner_edges, int tolerance){
    vector<Period> intel;
    Period period;

    int test[] = {0,5,11,17,21};
    //for each occurence vector of size k for pattern X repeat
    //for(int a=0;a<inner_edges.size();a++){
        for(int j=0;j<end(test)-begin(test)-1;j++){
            int sumPer = 0;
            int count = 0;
            int p = test[j+1] - test[j];
            int stPos = test[0];
            //int endPos = test[size(test)-1];
            int cmp = test[j];
            int currStPos = cmp;
            int preOccur = cmp+p; //makes it different
            for(int i=j;i<end(test)-begin(test);i++){
                double A = (double) test[i] - currStPos;
                double B = (double) round(A/p);
                cout.precision(3);
                cout << "shiet: " << test[i] << " B: " << B << endl;
                int C = A - (p*B);
                if( ((C>=(-1)*tolerance) && C<=tolerance) && (round((preOccur - currStPos)/p) != B) ){
                    currStPos = test[i];
                    preOccur = test[i];
                    sumPer = sumPer + p + C;
                    count++;
                }
                cout << "count: " << count << endl;
            }
            double meanP = (double) sumPer-(p/(count-1));
            double conf = (double) count/(floor((27-stPos+1)/p));
            /*
            if(conf >= 0.9){
                period.confidence = conf;
                period.period = p;
                intel.push_back(period);
            }
            */
            cout.precision(3);
            cout << " Confidence: " << conf << " count: " << count << " Perfect rep: "
            << floor((27-stPos+1)/p) <<
            " period: " << p <<endl;
            /*
            cout << "INNER EDGES: \n";
            cout << "Start Node: " << inner_edges[a].start_node << " End node: " << inner_edges[a].end_node << " Occurence vector: ";
            for(int occ = 0; occ<inner_edges[a].occur_vec.size(); occ++){
                cout << " " << inner_edges[a].occur_vec[occ];
            }
            cout << endl;
            */
        }
        cout << "JUMP" << endl;
    //}
    //write_file(intel, subject, channel);
}



//
// When a suffix ends on an implicit node, adding a new character
// means I have to split an existing edge.  This function is called
// to split an edge at the point defined by the Suffix argument.
// The existing edge loses its parent, as well as some of its leading
// characters.  The newly created edge descends from the original
// parent, and now has the existing edge as a child.
//
// Since the existing edge is getting a new parent and starting
// character, its hash table entry will no longer be valid.  That's
// why it gets removed at the start of the function.  After the parent
// and start char have been recalculated, it is re-inserted.
//
// The number of characters stolen from the original node and given
// to the new node is equal to the number of characters in the suffix
// argument, which is last - first + 1;
//

int Edge::SplitEdge( Suffix &s )
{
    Remove();
    Edge *new_edge =
      new Edge( first_char_index,
                first_char_index + s.last_char_index - s.first_char_index,
                s.origin_node );
    new_edge->Insert();
    Nodes[ new_edge->end_node ].suffix_node = s.origin_node;
    first_char_index += s.last_char_index - s.first_char_index + 1;
    start_node = new_edge->end_node;
    Insert();
    return new_edge->end_node;
}

//
// This routine prints out the contents of the suffix tree
// at the end of the program by walking through the
// hash table and printing out all used edges.  It
// would be really great if I had some code that will
// print out the tree in a graphical fashion, but I don't!
//


void dump_edges( int current_n )
{
    cout << " Start  End  Suf  First Last  String\n";
    for ( int j = 0 ; j < HASH_TABLE_SIZE ; j++ ) {
        Edge *s = Edges + j;
        if ( s->start_node == -1 )
            continue;
        cout << setw( 5 ) << s->start_node << " "
             << setw( 5 ) << s->end_node << " "
             << setw( 3 ) << Nodes[ s->end_node ].suffix_node << " "
             << setw( 5 ) << s->first_char_index << " "
             << setw( 6 ) << s->last_char_index << "  ";

        int top;
        if ( current_n > s->last_char_index )
            top = s->last_char_index;
        else
            top = current_n;
        for ( int l = s->first_char_index; l <= top; l++ )
            cout << T[ l ];
        cout << "\n";
    }
}

//
// A suffix in the tree is denoted by a Suffix structure
// that denotes its last character.  The canonical
// representation of a suffix for this algorithm requires
// that the origin_node by the closest node to the end
// of the tree.  To force this to be true, we have to
// slide down every edge in our current path until we
// reach the final node.

void Suffix::Canonize()
{
    if ( !Explicit() ) {
        Edge edge = Edge::Find( origin_node, T[ first_char_index ] );
        int edge_span = edge.last_char_index - edge.first_char_index;
        while ( edge_span <= ( last_char_index - first_char_index ) ) {
            first_char_index = first_char_index + edge_span + 1;
            origin_node = edge.end_node;
            if ( first_char_index <= last_char_index ) {
               edge = Edge::Find( edge.end_node, T[ first_char_index ] );
                edge_span = edge.last_char_index - edge.first_char_index;
            };
        }
    }
}

//
// This routine constitutes the heart of the algorithm.
// It is called repetitively, once for each of the prefixes
// of the input string.  The prefix in question is denoted
// by the index of its last character.
//
// At each prefix, we start at the active point, and add
// a new edge denoting the new last character, until we
// reach a point where the new edge is not needed due to
// the presence of an existing edge starting with the new
// last character.  This point is the end point.
//
// Luckily for use, the end point just happens to be the
// active point for the next pass through the tree.  All
// we have to do is update it's last_char_index to indicate
// that it has grown by a single character, and then this
// routine can do all its work one more time.
//

void AddPrefix( Suffix &active, int last_char_index )
{
    int parent_node;
    int last_parent_node = -1;

    for ( ; ; ) {
        Edge edge;
        parent_node = active.origin_node;
//
// Step 1 is to try and find a matching edge for the given node.
// If a matching edge exists, we are done adding edges, so we break
// out of this big loop.
//
        if ( active.Explicit() ) {
            edge = Edge::Find( active.origin_node, T[ last_char_index ] );
            if ( edge.start_node != -1 )
                break;
        } else { //implicit node, a little more complicated
            edge = Edge::Find( active.origin_node, T[ active.first_char_index ] );
            int span = active.last_char_index - active.first_char_index;
            if ( T[ edge.first_char_index + span + 1 ] == T[ last_char_index ] )
                break;
            parent_node = edge.SplitEdge( active );
        }
//
// We didn't find a matching edge, so we create a new one, add
// it to the tree at the parent node position, and insert it
// into the hash table.  When we create a new node, it also
//
// means we need to create a suffix link to the new node from
// the last node we visited.
        Edge *new_edge = new Edge( last_char_index, N, parent_node );
        new_edge->Insert();
        if ( last_parent_node > 0 )
            Nodes[ last_parent_node ].suffix_node = parent_node;
        last_parent_node = parent_node;
//
// This final step is where we move to the next smaller suffix
//
        if ( active.origin_node == 0 )
            active.first_char_index++;
        else
            active.origin_node = Nodes[ active.origin_node ].suffix_node;
        active.Canonize();
    }
    if ( last_parent_node > 0 )
        Nodes[ last_parent_node ].suffix_node = parent_node;
    active.last_char_index++;  //Now the endpoint is the next active point
    active.Canonize();
};

int main(int argc, char* argv[]){

    /*TODO:
        Check if the memory will go haywire - YAY!! i didn't went
    */
    std::clock_t start;
    start = std::clock();
    ifstream file;
    string file_name = "/home/gustavo/Documents/IC/Data";
    string type;
    //1 is Self-face and 0 is non-Self-face
    if(argc < 2) cout << "Missing arguments! There are two arguments: the subject and the option";

    string arg = argv[2];
    if(arg == "1"){
        type = "/SelfFaceSAX/SelfFaceSubject";
    }else{
        type = "/NonSelfFaceSAX/NonSelfFaceSubject";
    }
    string subject = argv[1];
    file_name = file_name+type+subject;

    Edge edges_initial_state[ HASH_TABLE_SIZE ];
    Node nodes_inicial_state[ MAX_LENGTH * 2 ];
    copy(std::begin(Edges), std::end(Edges), std::begin(edges_initial_state));
    copy(std::begin(Nodes), std::end(Nodes), std::begin(nodes_inicial_state));

    for(int f=1;f<=18;f++){
        cout << "Channel:" << f << endl;
        string channel = to_string(f);
        file_name = file_name+"Channel"+channel;
        file.open(file_name);
        string line;

        int lines = 0;
        vector<Period> intel;
        int periods_over_threshold = 0;
        int periods_string_size = 0;
        int total_periods_over_threshold = 0;
        int total_periods_string_size = 0;

        if (file.is_open()){
            while(getline(file,line)){
                //period vector


                Node::Count = 1;
                //reset the global variables
                copy(std::begin(edges_initial_state), std::end(edges_initial_state), std::begin(Edges));
                copy(std::begin(nodes_inicial_state), std::end(nodes_inicial_state), std::begin(Nodes));


                //getline(file,line);
                lines++;
                strcpy(T, line.c_str());
                T[line.size()] = '$';
                //cout << line;
                //cout << "STRING: "<< T << " LENGTH: " << strlen(T) << endl;


                N = strlen( T ) - 1;
                Suffix active( 0, 0, -1 );  // The initial active prefix
                for ( int i = 0 ; i <= N ; i++ ){
                    AddPrefix( active, i );
                }
                //if(lines == 1) break;
                intel = period_detection_with_tolerance(init_ocurrence_vector(), 1);
                if(intel.size()>0){
                    periods_over_threshold++;
                    total_periods_over_threshold++;
                }
                for(int q = 0; q<intel.size(); q++){
                    if(intel[q].sequence.size()>=5){
                        periods_string_size++;
                        total_periods_string_size++;
                        break;
                    }
                }
                if(lines%2000 == 0){
                    cout.precision(3);
                    cout << "Combination: " << setw(3) <<  lines/2000
                    << setw(1) << " Periods over threshold: " <<  setw(3) << periods_over_threshold << "/2000" <<" = "
                    << setw(6) << (double) periods_over_threshold/2000
                    << " String >= 5: "<< setw(4) << periods_string_size << "/2000"
                    << " = " << setw(6) <<(double) periods_string_size/2000 << endl;
                    periods_over_threshold = 0;
                    periods_string_size = 0;
                }


                /*
                //count edges to verify if the global variables aren't haywire
                int edge_count = 0;
                for(int u = 0; u<HASH_TABLE_SIZE;u++){
                    if(Edges[u].start_node != -1){
                        edge_count++;
                    }
                }
                */

                //cout << "line: "<< lines << endl;

                /*
                if(lines > 286450){
                    cout << T;
                    start = std::clock();
                    Suffix active( 0, 0, -1 );  // The initial active prefix
                    for ( int i = 0 ; i <= N ; i++ ){
                        AddPrefix( active, i );
                    }
                    init_ocurrence_vector();
                    //period_detection(init_ocurrence_vector(), subject, channel);
                }
                */
            }
        file.close();
        }else cout << "Unable to open file";
        cout.precision(3);
        cout << "Total periods over threshold: " << (double) total_periods_over_threshold/364000
        << "Total periods over 5:" << (double) total_periods_string_size/364000 << endl; 
    }

    /*
    std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
    cout << "Normally, suffix trees require that the last\n"
         << "character in the input string be unique.  If\n"
         << "you don't do this, your tree will contain\n"
         << "suffixes that don't end in leaf nodes.  This is\n"
         << "often a useful requirement. You can build a tree\n"
         << "in this program without meeting this requirement,\n"
         << "but the validation code will flag it as being an\n"
         << "invalid tree\n\n";
    cout << "Enter string: " << flush;
    cin.getline( T, MAX_LENGTH - 1 );
    N = strlen( T ) - 1;
    */
//
// The active point is the first non-leaf suffix in the
// tree.  We start by setting this to be the empty string
// at node 0.  The AddPrefix() function will update this
// value after every new prefix is added.
//
    /*
    Suffix active( 0, 0, -1 );  // The initial active prefix
    for ( int i = 0 ; i <= N ; i++ )
        AddPrefix( active, i );
    */
//
// Once all N prefixes have been added, the resulting table
// of edges is printed out, and a validation step is
// optionally performed.
//
    //abcdeabdcecabcdcaabddabbca$
    //period_detection_with_tolerance(init_ocurrence_vector(), 1);
    //init_ocurrence_vector();
    //dump_edges( N );
    //cout << strlen(T);
    /*
    cout << "Would you like to validate the tree?"
         << flush;
    std::string s;
    getline( cin, s );
    if ( s.size() > 0 && s[ 0 ] == 'Y' || s[ 0 ] == 'y' )
        validate();
    */
    return 1;
};

//
// The validation code consists of two routines.  All it does
// is traverse the entire tree.  walk_tree() calls itself
// recursively, building suffix strings up as it goes.  When
// walk_tree() reaches a leaf node, it checks to see if the
// suffix derived from the tree matches the suffix starting
// at the same point in the input text.  If so, it tags that
// suffix as correct in the GoodSuffixes[] array.  When the tree
// has been traversed, every entry in the GoodSuffixes array should
// have a value of 1.
//
// In addition, the BranchCount[] array is updated while the tree is
// walked as well.  Every count in the array has the
// number of child edges emanating from that node.  If the node
// is a leaf node, the value is set to -1.  When the routine
// finishes, every node should be a branch or a leaf.  The number
// of leaf nodes should match the number of suffixes (the length)
// of the input string.  The total number of branches from all
// nodes should match the node count.
//

char CurrentString[ MAX_LENGTH ];
char GoodSuffixes[ MAX_LENGTH ];
char BranchCount[ MAX_LENGTH * 2 ] = { 0 };

void validate()
{
    for ( int i = 0 ; i < N ; i++ )
        GoodSuffixes[ i ] = 0;
    walk_tree( 0, 0 );
    int error = 0;
    for ( int i = 0 ; i < N ; i++ )
        if ( GoodSuffixes[ i ] != 1 ) {
            cout << "Suffix " << i << " count wrong!\n";
            error++;
        }
    if ( error == 0 )
        cout << "All Suffixes present!\n";
    int leaf_count = 0;
    int branch_count = 0;
    for ( int i = 0 ; i < Node::Count ; i++ ) {
        if ( BranchCount[ i ] == 0 )
            cout << "Logic error on node "
                 << i
                 << ", not a leaf or internal node!\n";
        else if ( BranchCount[ i ] == -1 )
            leaf_count++;
        else
            branch_count += BranchCount[ i ];
    }
    cout << "Leaf count : "
         << leaf_count
         << ( leaf_count == ( N + 1 ) ? " OK" : " Error!" )
         << "\n";
    cout << "Branch count : "
         << branch_count
         << ( branch_count == (Node::Count - 1) ? " OK" : " Error!" )
         << endl;
}

int walk_tree( int start_node, int last_char_so_far )
{
    int edges = 0;
    for ( int i = 0 ; i < 256 ; i++ ) {
        Edge edge = Edge::Find( start_node, i );
        if ( edge.start_node != -1 ) {
            if ( BranchCount[ edge.start_node ] < 0 )
                cerr << "Logic error on node "
                     << edge.start_node
                     << '\n';
            BranchCount[ edge.start_node ]++;
            edges++;
            int l = last_char_so_far;
            for ( int j = edge.first_char_index ; j <= edge.last_char_index ; j++ )
                CurrentString[ l++ ] = T[ j ];
            CurrentString[ l ] = '\0';
            if ( walk_tree( edge.end_node, l ) ) {
                if ( BranchCount[ edge.end_node ] > 0 )
                        cerr << "Logic error on node "
                             << edge.end_node
                             << "\n";
                BranchCount[ edge.end_node ]--;
            }
        }
    }
//
// If this node didn't have any child edges, it means we
// are at a leaf node, and can check on this suffix.  We
// check to see if it matches the input string, then tick
// off it's entry in the GoodSuffixes list.
//
    if ( edges == 0 ) {
        cout << "Suffix : ";
        for ( int m = 0 ; m < last_char_so_far ; m++ )
            cout << CurrentString[ m ];
        cout << "\n";
        GoodSuffixes[ strlen( CurrentString ) - 1 ]++;
        cout << "comparing: " << ( T + N - strlen( CurrentString ) + 1 )
             << " to " << CurrentString << endl;
        if ( strcmp(T + N - strlen(CurrentString) + 1, CurrentString ) != 0 )
            cout << "Comparison failure!\n";
        return 1;
    } else
        return 0;
}
