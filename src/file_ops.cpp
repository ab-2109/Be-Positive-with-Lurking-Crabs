#include "../include/bptree.hpp"

void log_error(string_view msg)
{
    cerr<<"(ERROR) "<<msg<<endl;
    exit(EXIT_FAILURE);
}


string get_free_file_name(bool isData,int key)
{
    string fileName;
    if(isData)
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
    
map<int,string> BPlusTree::read_file(const string& fileName, bool& isLeaf)
{
    map<int,string> currNode;
    indexCache.read(fileName, currNode, &isLeaf);
    return currNode;
}


bool BPlusTree::write_file(const string& fileName,map<int,string>& currNode,bool isLeaf)
{
    return indexCache.write(fileName, currNode, isLeaf);
}
