#include<iostream>
#include<iomanip>
#include<fstream>
#include<cstring>
#include<algorithm>
#include<vector>
#include<map>
#include<set>
using namespace std;

const int MEMORY_SIZE=512;
const int LIST_SIZE=16;
const int SYM_SIZE=16;

ifstream file;
int line_cnt;
int cur_line;
int line_offset;
//int cur_off;
int char_cnt;
//int cur_cnt;
int module_cnt;
int module_addr;
char* this_line;
int cur_len;
char* sub_line=NULL;
string cur_str;
int pre;
int ins_num;
int def_or_use;

char* getToken();
void __parseerror(int errcode);
class Symbol {
    public:
        string sym;
        int module;
        bool dupli_flag;  
        bool used;
        int addr1;
        int addr2;
        int offset;
        int errcode;
        bool reset;
        Symbol(string str) {
            sym = string(str);
            module=0;
            dupli_flag = false;
            used=false;
            errcode=-1;
            addr1=0;
            addr2=0;
            offset=0;
            reset=false;
        }
        Symbol(Symbol* cur_sym) {
            sym = cur_sym->sym;
            module=cur_sym->module;
            addr1=cur_sym->addr1;
            addr2=cur_sym->addr2;

            dupli_flag = cur_sym->dupli_flag;
            used=false;
            if(cur_sym->errcode==3) errcode=3;
            else errcode=-1;
            offset=cur_sym->offset;
            reset=cur_sym->reset;
        }
};
vector<Symbol*> symbolTable;
vector<Symbol> useList;
vector<int> memoryMap;
vector<string> cur_errors;

bool subline_helper(char* sub_line){
    if(sub_line==NULL){
        //line_offset=sub_line-this_line+1;        
        return true;
    }
    //printf("now:%s,%s,%d\n",sub_line,this_line,line_offset);
    return false;
}
int offset_helper(char* sub_line){
    return sub_line-this_line+1;
}

char* getToken(){
    if(!subline_helper(sub_line)){
        sub_line=strtok(NULL," \t\n");
        if(subline_helper(sub_line)) {
            return getToken();
        }
        else {
            line_offset=offset_helper(sub_line);
            return sub_line;
        }
    }
    if(subline_helper(sub_line)){
        if(cur_str.length()>=0)
            pre=cur_str.size();
            getline(file,cur_str);
        if(!file.eof()){
            
            int now=cur_str.size();
            //cout<<now;
            this_line=const_cast<char*>(cur_str.c_str());
            sub_line=strtok(this_line," \t\n");
            line_cnt++;
            if(subline_helper(sub_line)) 
            {line_offset=now+1;
            return getToken();}
            else {
                //cout<<sub_line<<";"<<this_line;
                line_offset=offset_helper(sub_line);
                return sub_line;
            }
            
        }
        else{
            if(pre!=0) line_offset=pre+1;
            return NULL;
        }
    }
}
bool REIA_helper(char* token){
    //cout<<token;
    return (!strcmp(token,"R"))||(!strcmp(token,"E"))||(!strcmp(token,"I"))||(!strcmp(token,"A")); 
}
char* read_REIA(){
    char* cur=getToken();
    //cout<<cur<<"example_REIA";
    //cout<<line_cnt<<"\n";
    if(cur==NULL||!REIA_helper(cur)){
        //cout<<cur<<"\n";
        //if(!strcmp(cur,"R")) cout<<"should";
        //cout<<"line_cnt"<<line_cnt;
        __parseerror(2);
        exit(1);
    }
    //cout<<cur<<"example_REIA";
    return cur;
}
bool INT_helper(char* token){
    //cout<<token;
    char* d=token;
    for(;*d!=0;d++){
        if(!isdigit(*d)) return false;
    }
    return true;
}
int read_INT(int def_or_use){
    char* cur=getToken();
    if(cur==NULL||!INT_helper(cur)){
        if(def_or_use==1) return -1;
        if(def_or_use==0){
            __parseerror(0);
            exit(1);
        }
    }
    return atoi(cur);
}
bool symbol_helper(char* token){
    char*s =token;
    //cout<<*s;
    if(isalpha(*s)){
        //cout<<"start2";
        s++;
        for(;*s!=0;s++){
            if(!isalnum(*s)) {
                //cout<<*s;
                return false;
            }
        }
        return true;
        //cout<<"start";
    }
    return false;
}
Symbol* read_SYMBOL(){
    char* s=getToken();
    //cout<<s;
    Symbol* sym1=NULL;
    if(s==NULL||!symbol_helper(s)){
        //cout<<"offset"<<line_offset;
        __parseerror(1);
        exit(1);
    }
    else if(strlen(s)>SYM_SIZE){
        __parseerror(3);
        exit(1);
    }
    else{
        sym1=new Symbol(s);
        sym1->module=module_cnt;
        sym1->used=false;
        //return sym1;
    }
    return sym1;
}
void __parseerror(int errcode) 
{
	string errstr[] = {
		"NUM_EXPECTED",
		"SYM_EXPECTED",
		"ADDR_EXPECTED",
		"SYM_TOO_LONG",
		"TOO_MANY_DEF_IN_MODULE",
		"TOO_MANY_USE_IN_MODULE",
		"TOO_MANY_INSTR",
	};
    cout<<"Parse Error line "<< line_cnt <<" offset "<< line_offset<<": "<< errstr[errcode]<<"\n";
}
bool deflist_helper(){
    if(!file.eof()){
        __parseerror(0);
        exit(1);
    }
    return true;
}
void symbolTable_helper(Symbol* cur_sym){
    for(int i=0; i<symbolTable.size();i++){
        if(symbolTable[i]->sym==cur_sym->sym){
            symbolTable[i]->dupli_flag=true;
            symbolTable[i]->errcode=4;
            cur_sym->dupli_flag=true;
            cur_sym->errcode=4;
            break;
        }
    }
}
void passer_initializer(){
    line_cnt=0;
    line_offset=0;
    module_cnt=0;
    ins_num=0;
    module_addr=0;
    this_line=NULL;
    sub_line=NULL;
}
void passer1(char *fname){    
    int deflist_cnt;
    int uselist_cnt;
    int ins_cnt;
    vector<Symbol*> defineList;
    passer_initializer();
    file.open(fname);
    while(!file.eof()){
        module_cnt++;
        def_or_use=1;
        deflist_cnt=read_INT(def_or_use);
        if(deflist_cnt>LIST_SIZE){
            __parseerror(4);
            exit(1);
        }
        if(deflist_cnt==-1){
            if(deflist_helper()) break;
        }
        for(int i=0;i<deflist_cnt;i++){
            //vector<Symbol*> defineList;
            int sym_val;
            //cout<<"start";
            Symbol* cur_sym=read_SYMBOL();
            defineList.push_back(cur_sym);
            for(int i=0;i<symbolTable.size();i++){
            if(defineList[defineList.size()-1]->sym==symbolTable[i]->sym&&symbolTable[i]->reset){
                defineList[defineList.size()-1]->reset=true;
            }
            }
            //cout<<"end";
            def_or_use=0;
            sym_val=read_INT(def_or_use);
            symbolTable_helper(cur_sym);
            if(!cur_sym->dupli_flag){
                cur_sym->addr1=module_addr;
                cur_sym->addr2=module_addr+sym_val;
                cur_sym->offset=sym_val;
                //cout<<sym_val<<"\n";
                //cout<<cur_sym->addr2<<"\n";
                symbolTable.push_back(cur_sym);
            }
            /*const int off_2=symbolTable[symbolTable.size()-1]->offset;
            if(off_2>=ins_cnt){
                cout<<"Warning: Module "<< module_cnt<<": "<<cur_sym->sym.c_str()<<" too big "<<cur_sym->offset<<" (max="<<ins_cnt-1<<")"<<" assume zero relative\n";
                symbolTable[symbolTable.size()-1]->offset=0;
                symbolTable[symbolTable.size()-1]->addr2=module_addr;
            }*/
            /*if(cur_sym->offset>ins_cnt){
                cout<<"Warning: Module "<< module_cnt<<": "<<cur_sym->sym.c_str()<<" too big "<<cur_sym->offset<<" (max="<<ins_cnt-1<<")"<<" assume zero relative\n";
                 cur_sym->offset=0;
                 symbolTable[symbolTable.size()-1]->offset=0;
                 cur_sym->addr2=module_addr;
                 symbolTable[symbolTable.size()-1]->addr2=module_addr;
            }*/
        }
        def_or_use=0;
        uselist_cnt=read_INT(def_or_use);
        if(uselist_cnt>LIST_SIZE){
            __parseerror(5);
            exit(1);
        }
        for(int i=0;i<uselist_cnt;i++){
            read_SYMBOL();
        }
        ins_cnt=read_INT(def_or_use);
        if((ins_num+=ins_cnt)>MEMORY_SIZE){
            __parseerror(6);
            exit(1);
        }
        for(int i=0;i<ins_cnt;i++){
            //cout<<"line_cnt"<<line_cnt;
            read_REIA();
            read_INT(def_or_use);
        }
        /*for(int i=0;i<symbolTable.size();i++){
            if (symbolTable[i]->module==module_cnt&&symbolTable[i]->offset>ins_cnt) {
                 //cout<<symbolTable[i]->offset<<symbolTable[i]->sym<<"example_toobig\n";
                 cout<<"Warning: Module "<< module_cnt<<": "<<symbolTable[i]->sym.c_str()<<" too big "<<symbolTable[i]->offset<<" (max="<<ins_cnt-1<<")"<<" assume zero relative\n";
                 symbolTable[i]->offset=0;
                 symbolTable[i]->addr2=module_addr;
                 //i--;
             }
        }*/
        for(int j=0;j<defineList.size();j++){            
            for(int i=0;i<symbolTable.size();i++){
                //if(symbolTable[i]->sym=="ovZnXARozR"&&defineList[j]->sym=="ovZnXARozR")
                    //cout<<"ovZnXARozR"<<symbolTable[i]->offset<<"example_toobig"<<defineList[j]->reset<<"\t"<<defineList[j]->module<<"\t"<<module_cnt<<"\n";
            //if(defineList[j]->sym==symbolTable[i]->sym&&defineList[j]->reset){
                //symbolTable[i]->reset=true;
            //}
            //if(symbolTable[i]->sym=="ri7qcePCZe"&&defineList[j]->sym=="ri7qcePCZe")
                    //cout<<"ri7qcePCZe"<<symbolTable[i]->addr2-module_addr<<"example_toobig"<<symbolTable[i]->dupli_flag<<"\t"<<module_cnt<<"\n"<<defineList[j]->module;
            if (defineList[j]->sym==symbolTable[i]->sym&&defineList[j]->reset&&defineList[j]->module==module_cnt){
                const int off2=symbolTable[i]->addr2-module_addr;
                //if(symbolTable[i]->sym=="ovZnXARozR"&&defineList[j]->sym=="ovZnXARozR")
                    //cout<<symbolTable[i]->offset<<"example_toobig"<<symbolTable[i]->dupli_flag<<"\t"<<module_cnt<<"\n"<<defineList[j]->module;
                //if(symbolTable[i]->sym=="ovZnXARozR"&&defineList[j]->sym=="ovZnXARozR")
                    //cout<<symbolTable[i]->offset<<"example_toobig"<<symbolTable[i]->module<<"\t"<<module_cnt<<"\n"<<defineList[j]->module;
                if(off2>=ins_cnt)
                    cout<<"Warning: Module "<< module_cnt<<": "<<symbolTable[i]->sym.c_str()<<" too big "<<symbolTable[i]->offset<<" (max="<<ins_cnt-1<<")"<<" assume zero relative\n";
                //symbolTable[i]->offset=0;
                //symbolTable[i]->addr2=module_addr;
                
            }
                
            if (defineList[j]->sym==symbolTable[i]->sym&&symbolTable[i]->module==module_cnt&&symbolTable[i]->offset>=ins_cnt) {
                 //cout<<symbolTable[i]->offset<<symbolTable[i]->sym<<"example_toobig\n";
                 cout<<"Warning: Module "<< module_cnt<<": "<<symbolTable[i]->sym.c_str()<<" too big "<<symbolTable[i]->offset<<" (max="<<ins_cnt-1<<")"<<" assume zero relative\n";
                 symbolTable[i]->offset=0;
                 symbolTable[i]->addr2=module_addr;
                 defineList[j]->reset=true;
                 //i--;
             }
        }
        }
        module_addr+=ins_cnt;
    }
    cout << "Symbol Table\n";
    for(int i=0;i<symbolTable.size();i++){
        cout<<symbolTable[i]->sym<<"="<<symbolTable[i]->addr2;
        if(symbolTable[i]->dupli_flag){
            cout<<" Error: This variable is multiple times defined; first value used\n";
        }
        else cout<<"\n";
    }
    file.close();
}
bool uselist_helper(Symbol* cur_sym){
    for(int i=0;i<symbolTable.size();i++){
        //if(symbolTable[i]->sym==cur_sym->sym&&symbolTable[i]->module==cur_sym->module){
        if(symbolTable[i]->sym==cur_sym->sym){
            //cout<<cur_sym->sym<<"\texample\t"<<cur_sym->module;
            //symbolTable[i]->used=false;
            useList.push_back(new Symbol(symbolTable[i]));
            //if(cur_sym->module==symbolTable[i]->module)
                symbolTable[i]->used=true;
            //cout<<symbolTable[i]->sym<<symbolTable[i]->used<<"example_used2?\t"<<symbolTable[i]->module<<"\n";
            return true;
        }
    }
    return false;
}
int cnt_example=0;
int index_start=0;
void passer2(char *fname){
    file.clear();
    int deflist_cnt;
    int uselist_cnt;
    int ins_cnt;
    set<string> defined;
    map<string, bool> symbol_list_used;
    passer_initializer();
    file.open(fname);
    cout<<"Memory Map\n";
    while(!file.eof()){
        //cout<<"start";
        index_start+=memoryMap.size();
        memoryMap.clear();
        
        module_cnt++;
        def_or_use=1;
        deflist_cnt=read_INT(def_or_use);
        if(deflist_cnt==-1){
            if(file.eof()) break;
        }
        def_or_use=0;
        for(int i=0;i<deflist_cnt;i++){

            Symbol* cur_sym=read_SYMBOL();
            read_INT(def_or_use);
            symbol_list_used[cur_sym->sym];
            defined.insert(cur_sym->sym);
        }
        
        useList.clear();
        uselist_cnt=read_INT(def_or_use);
        //cout<<"attention:"<<uselist_cnt;
        for(int i=0;i<uselist_cnt;i++){
            Symbol* cur_sym=read_SYMBOL();
            if(!uselist_helper(cur_sym)){
                cur_sym->errcode=3;
                symbolTable_helper(cur_sym);
                if(!cur_sym->dupli_flag){
                    cur_sym->addr1=module_addr;
                    cur_sym->addr2=cur_sym->addr1;
                    cur_sym->offset=0;
                //cout<<sym_val<<"\n";
                //cout<<cur_sym->addr2<<"\n";
                    symbolTable.push_back(cur_sym);
                }
                for(int j=0;j<symbolTable.size();j++) {
					if(symbolTable[j]->sym==cur_sym->sym) {
                        //cout<<cur_sym->sym<<cur_sym->module<<"\texample_used_3\n";
                        //cout<<symbolTable[j]->sym.c_str()<<<<"example\n";
						useList.push_back(new Symbol(symbolTable[j]));
                        //cout<<useList[useList.size()-1].used<<useList[useList.size()-1].sym<<"example\n";
                        //if(symbolTable[j]->offset==0)
                        //    useList[useList.size()-1].used=true;
                        //cout<<useList[useList.size()-1]->used<<"used?"<<"\n";
                        //if(cur_sym->module==symbolTable[j]->module)
						    symbolTable[j]->used=true;
                        //useList[useList.size()-1]->used=false;
                        //cout<<useList[useList.size()-1].used<<"used?"<<"\n";
                        //cout<<symbolTable[j]->sym<<symbolTable[j]->used<<"example_used?\t"<<symbolTable[j]->module<<"\n";
						break;
					}
				}
                
                //uselist_helper(cur_sym);
            }
        }
        /*for(int j=0;j<useList.size();j++) {
			cout<<"useList:"<<useList[j].sym<<"\n";
		}*/
        //cout<<"end";
        ins_cnt=read_INT(def_or_use);
        for(int i=0;i<ins_cnt;i++){
            string cur_err="";
            char* addr_type_tmp=read_REIA();
            char addr_type=addr_type_tmp[0];
            //cout<<addr_type<<": "<<"example_type\n";
            int ins=read_INT(def_or_use);
            int opcode=ins/1000;
            int operand=ins%1000;
            //cout<<addr_type<<": "<<"example_type2\n";
            if(addr_type=='I'){
                if(ins>=10000){
                    opcode=9;
                    operand=999;
                    cur_err = " Error: Illegal immediate value; treated as 9999";
                }
            }
            else if(opcode>=10){
                opcode=9;
                operand=999;
                cur_err = " Error: Illegal opcode; treated as 9999";
            }
            else if(addr_type=='E'){
                //cout<<"attention"<<operand;
                cnt_example++;
                if(operand>=uselist_cnt){
                    cur_err = " Error: External address exceeds length of uselist; treated as immediate";
                }
                else if(ins>=10000){
                    opcode=9;
                    operand=999;
                    cur_err = " Error: Illegal opcode; treated as 9999";
                }
                else if(useList[operand].errcode==3){
                    cur_err=" Error: " + useList[operand].sym + " is not defined; zero used";
                    useList[operand].used=true;
                    //cout<<useList[operand].sym<<useList[operand].used<<"example\n";
                    operand=0;
                }
                else{
                    useList[operand].used=true;
                    //cout<<useList[operand].sym<<useList[operand].addr2<<"example\n";
                    operand=useList[operand].addr2;
                    //cout<<operand<<"example2\n";
                }
            }
            else if(addr_type=='R'){
                if(operand>=ins_cnt){
                    operand=0;
                    cur_err=" Error: Relative address exceeds module size; zero used";
                }
                operand+=module_addr;
            }
            else if(addr_type=='A'){
                if(operand>=MEMORY_SIZE){
                    operand=0;
                    cur_err=" Error: Absolute address exceeds machine size; zero used";
                }
            }
            //cout<<operand<<"example3\n";
            int cur_op=opcode*1000+operand;
            //cout<<cur_op<<"example2";
            memoryMap.push_back(cur_op);
            //cout<<cur_err;
            cur_errors.push_back(cur_err);
        }
        /*for(int i=0;i<cur_errors.size();i++){
            cout<<i<<":"<<cur_errors[i]<<"\n";
        }*/
        for(int i=0;i<memoryMap.size();i++){
            //cout<<cur_errors[i];
            /*if(i+index_start==7||i+index_start==11){
                cout<<memoryMap[i]<<"example";
            }*/
            cout<<setw(3)<<setfill('0')<<i+index_start<<": "<<setw(4)<<setfill('0')<<memoryMap[i]<<cur_errors[i+index_start]<<"\n";
        }
        for(int i=0;i<useList.size();i++){
            //cout<<useList[i].sym.c_str()<<useList[i].errcode<<useList[i].used<<"example\n";
            //if(!useList[i].used&&useList[i].errcode!=3){
            symbol_list_used[useList[i].sym]|=useList[i].used;
            if(!useList[i].used){
                //cout<<useList[i].errcode<<useList[i].sym;
                cout<<"Warning: Module "<< module_cnt<<": "<<useList[i].sym.c_str()<<" appeared in the uselist but was not actually used\n";
            }
        }
        module_addr+=ins_cnt;
    }
    for(int i=0;i<symbolTable.size();i++){
        //cout<<symbolTable[i]->sym<<"example:"<<symbolTable[i]->used<<"\n";
        
        //if(!symbolTable[i]->used){
        if(defined.find(symbolTable[i]->sym)!=defined.end()&&!symbol_list_used[symbolTable[i]->sym]){
            
            cout<<"Warning: Module "<< symbolTable[i]->module<<": "<<symbolTable[i]->sym.c_str()<<" was defined but never used\n";
        }
    }
    //cout<<cnt_example<<"example_cnt\n";
    cout<<"\n";
    file.close();
}
int main(int argc, char** argv){
    char* fname=argv[1];
    //file.open(fname);`
    //while(!file.eof()){
    //Symbol* x=read_SYMBOL();
    //char* examp=read_REIA();
    //cout<<x->sym<<"\t";
    //cout<<line_cnt<<"\t";
    //cout<<line_offset<<"\t";
    //cout<<"\n\n";
    /*printf("%d\t",linenum);
    printf("%d\t",lineoffset);*/
    //printf("\n\n");
    //}
    passer1(fname);
    passer2(fname);
    return 0;
}