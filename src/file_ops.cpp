#include "../include/bptree.hpp"
#include <map>


template<typename StreamType>
inline void file_opened(const StreamType& file,string_view fileName)
{
    if(file.is_open()==false)
    {
        cerr<<"(ERROR) file_ops.cpp: read_file: Can't Open file "<<fileName<<endl;
        exit(EXIT_FAILURE);
    }
}

inline void log_error(string_view msg)
{
    cerr<<"(ERROR) "<<msg<<endl;
    exit(EXIT_FAILURE);
}


string get_free_file_name(bool isLeaf,int key=-1)
{
    string fileName;
    if(isLeaf)
    {
        fileName="Data/"+to_string(key)+".txt";
    }
    else{
        if(BPlusTree::availPointer.empty())
        log_error("file_ops.cpp: get_free_file_name(): Not key free");

        key = BPlusTree::availPointer.top();
        BPlusTree::availPointer.pop();
        fileName = "Index/" + to_string(key) + ".txt";
    }

    return fileName;
}
    
map<int,string> read_file(string& fileName,bool& isLeaf)
{
    ifstream file(fileName,ios::in);
    file_opened(file,fileName);

    map<int,string> currNode;

    file>>isLeaf;
    string filePtrs;


    if(isLeaf==false)   // due to this we need to store it as 
    {
        file>>filePtrs;
        currNode[0]=filePtrs;
    }
        
    int key;
    while(file>>key>>filePtrs)
    {
        currNode[key]=filePtrs;
    }
    
    return currNode;
}


bool write_file(string& fileName,map<int,string>& currNode,bool isLeaf)
{
    ofstream file(fileName,ios::out);
    file_opened(file,fileName);

    file<<isLeaf;

    if(!isLeaf)
    {
        if(currNode.find(0)==currNode.end())
            log_error("file_ops.cpp: write_file(): Not found currNode[0] in non Leaf node");

        file<<currNode[0];
    }

    for(const auto&[key,filePtr]:currNode)
    {
        if(!isLeaf && key==0)
            continue;

        file<<key<<filePtr;
    }

    return true;
}
