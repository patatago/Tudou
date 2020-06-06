#include <iostream>

struct node
{
    node(int d) { data = d; }
    int data;
    struct node *left, *right, *parent;
};

using PBinTree = node;
PBinTree *root;
PBinTree * temp;
// 
void func(PBinTree *root)
{
    
    if(root == temp)
    {
        root->parent = nullptr;
    }
    else
    {
        root->parent = temp;
    }

    if(root->left)
    {   
        temp = root;
        func(root->left);
    }
    if(root->right) 
    {
        temp = root;
        func(root->right);
    }
}
void test(PBinTree *root)
{
    if(root)
    {
        if(root->parent) std::cout << root->data << "____" << root->parent->data;
        else std::cout << root->data << "____" << 0 << std::endl;;
    }
    if(root->left) test(root->left);
    if(root->right) test(root->right);
}
node * createTree()
{
   
    if(root == nullptr) root = new node(1);
    auto first = root;
    root->left = new node(2), root->right = new node(3);
    auto tem = root;
    root =root->left;
    root->left = new node(4);
    root = root->left;
    root->right = new node(6);
    tem->right = new node(5);
    return first;
}
int main()
{
  
    root = createTree();
    temp = root;
    func(root);
    test(root);
}