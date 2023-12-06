#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>

using namespace std;

//class to represent state
class State {
public:
    bool isFinal; //whether the current state is final state
    bool isInitial; //whether the current state is initial state
    int id; // state ID
    map<char,vector<State*>> transitions; //stores transition for the current state
	
	//constructor
    State(int id,bool final = false, bool initial = false) :id(id), isFinal(final), isInitial(initial) {}
};


//class to construct enfa
class ENFA {
public:
    int stateCount = 0; //to keep track of state
    int charCounter = 0; //to keep track of the inputs for the nfa
    vector<State*> states;  //to store all the states
    map<char,int> hash; // to store all characters (helps in TT)
    
    // default cunstrutor
    ENFA() {

    }
	
	//add transition from one state to another (default epsilon transtion)
    void addTransition(State* from, State* to, char symbol = 'e') {
        from->transitions[symbol].push_back(to);
    }

    //add a state and push it in the states vector
    State* addState(bool isFinal = false, bool isInitial = false) {
        State* state = new State(stateCount++,isFinal);
        states.push_back(state);
        return state;
    }
	
	//this will help in building Transition Table
    bool isNotInMap(const map<char, int>& myMap, char key) {
        return myMap.find(key) == myMap.end();
    }

    void convert_to_table(string** arr){
        int i = 0;
        string str;
        int index;
        int count = 0;
        for(auto& s: states){
            arr[i][0] = to_string(s->id);
            for(auto& t:s->transitions){
                for(auto& p:t.second){
                    count++;
                    if(count>1){
                        str = str + ","+ to_string(p->id);
                    }
                    else
                        str = str + to_string(p->id);
                }
                count =0;
                index = hash[t.first];
                arr[i][index] = str;
                str = "";
            }
            i++;
            if(i==stateCount-1) break;
        }
    }
	
	//Destructor for deallocating all the allocated memory for states
    ~ENFA() {
        for (State* state : states) {
            delete state;
        }
    }
    
    
};

//logic for converting regex to e-nfa
void regexToNfa(string& regex, ENFA& enfa) {
	
    stack<State*> stack; // used to keep track of previos start state and end state 
    
    //iterate over regex 
    for (int i=0;i<regex.length();i++) {
    	
    	// if there is single character 
        if (isalpha(regex[i]) && regex[i+1]!='.') {
            State* start = enfa.addState();
            State* end = enfa.addState();
            enfa.addTransition(start, end, regex[i]);
            stack.push(start);
            stack.push(end);
			
			//push the character if it is not marked
            if(enfa.isNotInMap(enfa.hash,regex[i])){
                enfa.charCounter++;
                enfa.hash[regex[i]] = enfa.charCounter;
            }         
        } 
        
        //logic for other input character
		else {
			
            /*logic to handle normal concatenation
             	1)create new state
            	2)pop the start and end of the previos transition
            	3)connect end to the new state with current symbol(regex[i])
            	4)push start and new state back to stack
            */
            
            if (isalpha(regex[i]) && regex[i+1]== '.') {
            	
                //Normal Concatenation
                State* end1 = stack.top(); stack.pop();
                State* start1 = stack.top(); stack.pop();
                State* end = enfa.addState();
                enfa.addTransition(end1, end,regex[i]);
                stack.push(start1);
                stack.push(end);
				
				//push the character if it is not marked
                if(enfa.isNotInMap(enfa.hash,regex[i])){
                    enfa.charCounter++;
                    enfa.hash[regex[i]] = enfa.charCounter;
                }
                i++;
            }
            
			/* Logic to handle concatination
				1) pop the start and end of both the previos transitions
				2) concatenate them with epsilon transion 
				3) push start of first and end of second to the stack
			*/ 
			
            else if (regex[i] == '.') {
                // Concatenation
                State* end1 = stack.top(); stack.pop();
                State* start1 = stack.top(); stack.pop();
                State* end2 = stack.top(); stack.pop();
                State* start2 = stack.top(); stack.pop();

                enfa.addTransition(end2, start1);
                stack.push(start2);
                stack.push(end1);
            }
            
            /*Logic to handle union
            	1) pop the start and end of both previos transitions
            	2) create new start and end state
            	3) make an epsiolon transition from new start to both starts
            	4) make an epsilon transiton from both ends to new end
            	5) push new start and new end to the stack
			*/ 
			
            else if (regex[i] == '|') {
                // Union
                State* end1 = stack.top(); stack.pop();
                State* start1 = stack.top(); stack.pop();
                State* end2 = stack.top(); stack.pop();
                State* start2 = stack.top(); stack.pop();

                State* start = enfa.addState();
                State* end = enfa.addState();
                enfa.addTransition(start, start1);
                enfa.addTransition(start, start2);
                enfa.addTransition(end1, end);
                enfa.addTransition(end2, end);

                stack.push(start);
                stack.push(end);
            }
            
			/* logic for kleene star
				1) pop the start and end of the previous transition 
				2) create new start and end state
				3) make an epsilon transition from end to start
				4) make an epsilon from new start to start and new end
				5) make an epsilon from end to new end
				6) push new start and new end to stack
			*/
			
			else if (regex[i] == '*') {
                State* end = stack.top(); stack.pop();
                State* start = stack.top(); stack.pop();

                State* newStart = enfa.addState();
                State* newEnd = enfa.addState();
                enfa.addTransition(newStart, start);
                enfa.addTransition(end, start);
                enfa.addTransition(end, newEnd);
                enfa.addTransition(newStart, newEnd);
                
                stack.push(newStart);
                stack.push(newEnd);
            }
            else if (regex[i] == '+') {
                State* end = stack.top(); stack.pop();
                State* start = stack.top(); stack.pop();

                State* newStart = enfa.addState();
                State* newEnd = enfa.addState();
                enfa.addTransition(newStart, start);
                enfa.addTransition(end, start);
                enfa.addTransition(end, newEnd);
//                enfa.addTransition(newStart, newEnd);
                
                stack.push(newStart);
                stack.push(newEnd);
            }
        }
    }

    // Set the final state of the NFA
    if (!stack.empty()) {
        State* endState = stack.top(); stack.pop();
        endState->isFinal = true;
        // enfa.startState = stack.top(); stack.pop();
        State* startState = stack.top(); stack.pop();
        startState->isInitial = true;
		enfa.hash['e'] = ++enfa.charCounter;
    }
}

//for precedenc (used for converting infix to postfix)
int precedence(char c) {
    if (c == '*') {
        return 3;
    } else if (c == '.') { // Represent concatenation explicitly with '.'
        return 2;
    } else if (c == '|') {
        return 1;
    } else {
        return -1;
    }
}

// Function to convert infix regex to postfix
string infixToPostfix(const string& regex) {
    stack<char> stack;
    string postfix;

    for (char c : regex) {
        if (c == '(') {
            stack.push(c);
        } else if (c == ')') {
            while (!stack.empty() && stack.top() != '(') {
                postfix += stack.top();
                stack.pop();
            }
            if (!stack.empty()) {
                stack.pop(); // Pop the '('
            }
        } else if (c == '*' || c == '|' || c == '.') {
            while (!stack.empty() && precedence(c) <= precedence(stack.top())) {
                postfix += stack.top();
                stack.pop();
            }
            stack.push(c);
        } else {
            postfix += c;
        }
    }

    while (!stack.empty()) {
        postfix += stack.top();
        stack.pop();
    }

    return postfix;
}



int main() {
    string regex;
    cout << "Enter a regular expression: ";
    cin >> regex;
    regex = infixToPostfix(regex); //converting to postfix
    // cout<<regex<<endl; //to check postFix
    ENFA enfa;//initaiting object of enfa
    
    regexToNfa(regex, enfa);

    //code related to displaying the resulted E-NFA
    cout<<"tree representation of the NFA\n\n";
    int initial = -1;
    int final = -1;
    for(auto& states: enfa.states) {
        cout<<states->id;
        if(states->isInitial){
            initial = states->id;
        }
        if(states->isFinal){
            final = states->id;
        }
        for(auto& transition:states->transitions){
            cout<<transition.first<<"->";
            for(auto& i:transition.second){
                cout<<i->id<<",";
            }
        }
        cout<<endl;
    }
	
    string arr[final][enfa.charCounter];
    for(int i = 0;i<final;i++){
        for(int j=0;j<enfa.charCounter;j++){
            arr[i][j] = "-";
        }
    }
    

    cout<<"\ndetails of the NFA\n\n";
    cout<<"Initial State:"<<initial<<endl;
    cout<<"Final State:"<<final<<endl;
    cout<<"Number of input Charecters = "<<enfa.charCounter<<endl;
    cout<<"Input Charecters:";
    for(auto& cha:enfa.hash){
        cout<<cha.first<<", ";
    }
    cout<<endl;

    //decalring and initializing the dynamic array for transition table	
    string** Transition_table = new string*[final];
	for(int i =0;i<final;i++){
        Transition_table[i] = new string[enfa.charCounter+1]; 
		for(int j=0;j<enfa.charCounter+1;j++){
			Transition_table[i][j] = "-";
		}
	}

    //convert to transtion table the given tree
    enfa.convert_to_table(Transition_table);

    //printing the transition table
    cout<<"\n\nThe transition table for the given regex\n"<<endl;
    cout<<"states\t";
    for(auto& cha:enfa.hash){
        cout<<cha.first<<"\t";
    }
    cout<<endl;
	for(int i =0;i<final;i++){
		for(int j=0;j<enfa.charCounter+1;j++){
			cout<<Transition_table[i][j]<<"\t";
		}
		cout<<endl;
	}

    return 0;
}
