// Struct pointers

typedef struct Node Node;

struct Node {
    int x;
    Node* parent;
};

int main() {
    Node node;
    Node child;
    Node* node_ptr = &node;
    Node* child_ptr = &child;
    child_ptr->parent = node_ptr;
    return child_ptr->parent == node_ptr;
}