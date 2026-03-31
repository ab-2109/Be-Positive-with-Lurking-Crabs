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
    // ifstream file(fileName,ios::in);
    // file_opened(file,fileName);

    // map<int,string> currNode;

    // if(!(file>>isLeaf))
    //     log_error("file_ops.cpp: read_file(): Corrupt file header in " + fileName);
    // string filePtrs;


    // if(isLeaf==false)   // due to this we need to store it as 
    // {
    //     if(!(file>>filePtrs))
    //         log_error("file_ops.cpp: read_file(): Missing leftmost pointer in " + fileName);
    //     currNode[0]=filePtrs;
    // }
        
    // int key;
    // while(file>>key>>filePtrs)
    // {
    //     currNode[key]=filePtrs;
    // }
    
    // return currNode;
    map<int,string> currNode;
    indexCache.read(fileName, currNode, &isLeaf);
    return currNode;
}


bool BPlusTree::write_file(const string& fileName,map<int,string>& currNode,bool isLeaf)
{
    // ofstream file(fileName,ios::out);
    // file_opened(file,fileName);

    // file<<isLeaf<<endl;

    // if(!isLeaf)
    // {
    //     if(currNode.find(0)==currNode.end())
    //         log_error("file_ops.cpp: write_file(): Not found currNode[0] in non Leaf node");

    //     file<<currNode[0]<<endl;
    // }

    // for(const auto&[key,filePtr]:currNode)
    // {
    //     if(!isLeaf && key==0)
    //         continue;

    //     file<<key<<endl<<filePtr<<endl;
    // }

    // return true;
    return indexCache.write(fileName, currNode, isLeaf);
}
