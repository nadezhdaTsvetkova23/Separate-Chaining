#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator;
  //using key_compare = std::less<key_type>;                         // B+-Tree
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

private:
  size_type table_size{0}; //size of the table - num of nodes
  size_type current_size{0}; //all the elements in the hash table at the moment
  float max_lfactor{0.7}; //maximum load factor

  struct Node{
    key_type node_value;
    Node* next{nullptr};
    Node* head{nullptr};
    ~Node(){
      delete next;
      delete head;
    }
  };
  Node erasedNode;
  Node* table{nullptr}; 

  size_type hash_i(const key_type &key) const { return hasher{}(key) % table_size; }
  Node* locate(const key_type& key) const;
  Node* add(const key_type &key);
  void reserve(size_type n);
  void rehash(size_type n);

public:
// ---Constructors---

  ADS_set(){  
    rehash(N);
    // table = new Node[table_size];
  }                                                        
  
  ADS_set(std::initializer_list<key_type> ilist): ADS_set() {
    insert(ilist);
  }
                        
  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set() {
    insert(first, last);
  }

  ADS_set(const ADS_set &other) {
    rehash(other.table_size);
    for(const auto& key: other){
      this->add(key);
    }
  }

  ~ADS_set(){
    delete[] table;
  }

  ADS_set &operator=(const ADS_set &other);
  ADS_set &operator=(std::initializer_list<key_type> ilist);

// ---Methods---
  size_type size() const{ return current_size; }                                             
  bool empty() const{ return current_size == 0;}                                                 

  void insert(std::initializer_list<key_type> ilist){
    insert(std::begin(ilist), std::end(ilist));
  }                  
  std::pair<iterator,bool> insert(const key_type &key);
  template<typename InputIt> void insert(InputIt first, InputIt last); 

  void clear(){
    //delete[] table;
    ADS_set help;
    this->swap(help);
  }
  size_type erase(const key_type &key);

  size_type count(const key_type &key) const{
    return this->locate(key) != nullptr;
  }                          
  iterator find(const key_type &key) const;

  void swap(ADS_set &other);

  const_iterator begin() const {
    if(current_size == 0)
      return end();

    for(size_type i{0}; i < table_size; i++)
      if(table[i].head != nullptr)
        return iterator(this, table[i].head, i, table_size, false);
      
    return end();
  }

  const_iterator end() const{
    return iterator();
  }

  void dump(std::ostream &o = std::cerr) const;

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs){
    if(lhs.current_size != rhs.current_size)
      return false;
    for(const auto& k: lhs){
      if(!rhs.count(k))
        return false;
    }

    return true;
  }
  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs){
    return !(lhs == rhs);
  }

  const_iterator y() const { 
    if(current_size < 2)
      return end();

    for(size_type i{0}; i < table_size; i++)
      if(table[i].head != nullptr)
        return iterator(this, table[i].head, i, table_size, true);
      
    return end(); 
  }
};

template <typename Key, size_t N>
typename ADS_set<Key,N>::Node *ADS_set<Key,N>::locate(const key_type &key) const{
  size_type index {hash_i(key)};
  Node* node_return{nullptr};
  if(table[index].head == nullptr)   //if the element with the specified key is missing 
    return nullptr;
  else if(table[index].head != nullptr){
    node_return = table[index].head;
    while(node_return != nullptr){
      if(key_equal{}(node_return->node_value, key))
        return node_return;
      node_return = node_return->next;
    }
    node_return = nullptr;
    delete node_return;
  }
  return nullptr;

}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Node *ADS_set<Key,N>::add(const key_type &key)  {
  
  Node* help = new Node; //initialize temporary helper node
  size_type index {hash_i(key)}; // using hash funktion
  
  if(table[index].head != nullptr){
    help->next = table[index].head;
  } //else {}
  help->node_value = key; 

  this->table[index].head = help;
  current_size++;
  
  reserve(current_size);
  
   help = nullptr;
   delete help;
   return &table[index];
}

template <typename Key, size_t N>
template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last){
  for(auto it = first; it != last; it++){
    if(!locate(*it)){
      reserve(current_size + 1);
      add(*it);
    }
  }
}

template <typename Key, size_t N>
void ADS_set<Key,N>::reserve(size_type n) {

  if(table_size * 2 >= n)
    return;
    
  size_type new_size{table_size};
  do{
    new_size = new_size * 4 + 1;
  } while(n > new_size * 4);
  rehash(new_size);  
}

template <typename Key, size_t N>
void ADS_set<Key, N>::rehash(size_type n){
  
  std::vector<key_type> myVec; //we are using vector for storing all of our existing nodes before rehashing them to new positions
	size_type new_table_size {std::max(N,std::max(n,size_type(current_size/max_lfactor)))};
  Node* new_table {new Node[new_table_size]};
  Node* old_table {table};
  size_type old_table_size {table_size};


	for (size_type idx{ 0 }; idx < old_table_size; ++idx) {
		for (Node* node{ table[idx].head }; node != nullptr; node = node->next) {
			myVec.push_back(node->node_value);
		}
	}

  current_size = 0;
  table = new_table;
  table_size = new_table_size;

	for (size_type idx{ 0 }; idx < myVec.size(); ++idx) {
		this->add(myVec[idx]);
	}

	delete[] old_table; //free the allocated memory
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(const ADS_set &other){
  if(this == &other)
    return *this;
  ADS_set help{other};
  this->swap(help);
  return *this;
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(std::initializer_list<key_type> ilist){
  ADS_set help{ilist};
  this->swap(help);
  return *this;
}

template <typename Key, size_t N>
void ADS_set<Key,N>::swap(ADS_set &other){
  using std::swap;

  swap(table, other.table);
  swap(table_size, other.table_size);
  swap(current_size, other.current_size);
  swap(max_lfactor, other.max_lfactor);
}

template <typename Key, size_t N>
std::pair<typename ADS_set<Key,N>::iterator,bool> ADS_set<Key,N>::insert(const key_type &key){
  if(auto l {locate(key)})      //if the element with the key is already in the table, can't be added -> false
    return std::make_pair(find(key), false); 
  reserve(current_size*2);      
  this->add(key);

  return std::make_pair(find(key), true); 
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::size_type ADS_set<Key,N>::erase(const key_type &key){
  if(auto n {locate(key)}){
    erasedNode = n;
    size_type node_index = hash_i(key);
    Node* help_node1 = table[node_index].head;
    Node* help_node2 = nullptr;

    if(key_equal{}(help_node1->node_value, key)){
      table[node_index].head =help_node1->next;
      help_node1->next= nullptr;
      delete help_node1; //free the memory on the deleted place
      //help_node2 = nullptr;
      current_size--;
      return 1;
    }
    else{
      help_node2 = help_node1->next;
      while(help_node2 != nullptr){
        Node *help3 = help_node1;
        help_node1 = help_node1->next;
        if(key_equal{}(help_node1->node_value, key)){
          help3->next = help_node1->next;
          help_node1->next = nullptr;
          help3 = nullptr;
          delete help3;
          delete help_node1;
         // delete help_node2; //?
          current_size--;
          return 1;
        }
        help_node2 = help_node2->next;
      }
    }

  }

  return 0;
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::iterator ADS_set<Key,N>::find(const key_type &key) const{
  if(auto n{locate(key)})
    return iterator{this, n, hash_i(key), table_size};
  return end();
}

//---Debuging-View---
template <typename Key, size_t N>
void ADS_set<Key,N>::dump(std::ostream &o) const{
  Node* node{nullptr};

  o << "***Hash Table: Separate Chaining***" << std::endl;
  o << "table_size = " << table_size << ", current_size = " << current_size << std::endl;

  for(size_type i{0}; i < table_size; i++){
    o << i << ": ";           //show number of the element
    node = table[i].head;
    while(node != nullptr){
      o << "--->" << node->node_value;
      node = node->next;
    }
    if(node == nullptr){
      o << "-" << std::endl;
    }
  }
  delete node;
}

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
  
  const ADS_set<Key, N>* mySet;
  Node* node;
  size_type first_index_iter; 
  size_type table_size_iter; 
  void skip() {
    while(first_index_iter < table_size_iter && mySet->table[first_index_iter].head == nullptr) {
        ++first_index_iter;
    }
    if(first_index_iter < table_size_iter) {
        node = mySet->table[first_index_iter].head;
    }
  }
  bool special;
  size_type n;
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit Iterator(const ADS_set<Key, N>* mySet = nullptr, Node* node = nullptr, size_type first_index_iter = 0, size_type table_size_iter = 0, bool special = false): 
  mySet(mySet), node(node), first_index_iter(first_index_iter), table_size_iter(table_size_iter), special(special), n{1}{
    if(!this->node)
      return;
  }
  
  ~Iterator(){
    node = nullptr;
    mySet = nullptr;
    delete node;
    delete[] mySet;
  }

  reference operator*() const{
    return node->node_value;
  }

  pointer operator->() const{
    return &node->node_value;
  }
  
  Iterator &operator++(){
    node = node->next;
    if(node == nullptr){
      ++first_index_iter;
      skip();
    } 
    return *this;

  }

  Iterator operator++(int){
    auto ret {*this};
    ++*this;
    return ret;
  }
  friend bool operator==(const Iterator &lhs, const Iterator &rhs){
    return lhs.node == rhs.node;
  }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs){
    return !(lhs==rhs);
  }
};

 template <typename Key, size_t N>
 void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H