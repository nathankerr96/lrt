#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>


typedef struct Point {
    int *components;
}Point;


typedef struct node {
    Point point;
    struct node *left_child;
    struct node *right_child;
    struct rangeTree *rt;
}Node;


typedef struct rangeTree {
    int size;
    int dimensions;
    Node *root;
}RangeTree;


typedef struct LRT {
    int size;
    int dimensions;
    Node *root;
}LRT;



void print_point(Point p, int dimensions);
RangeTree *build_range_tree(Point *points, int size, int dimension, int total_dimensions);
int check_subtree_ordering(Node *root, int current_dimension, int min, int max);
int check_range_tree_ordering(Node *root, int current_dimension, int total_dimensions);
int check_range_subtrees(Node *root, int current_dimension, int total_dimensions);

//function to generate random points in d dimensions
Point* generate_random(int to_generate, int dimensions) {
    srand(time(NULL));

    Point *points = malloc(sizeof(Point)*to_generate);
    int i;
    for (i=0; i<to_generate; i++) {
        points[i].components = malloc(sizeof(int)*dimensions);
        int j;
        for (j=0; j<dimensions; j++) {
            points[i].components[j] = rand() % 1000000;
        }
    }

   return points; 
}


Point* generate_known(int to_generate, int dimensions) {
    srand(time(NULL));

    Point *points = malloc(sizeof(Point)*to_generate);
    int i;
    for (i=0; i<to_generate; i++) {
        points[i].components = malloc(sizeof(int)*dimensions);
        int j;
        for (j=0; j<dimensions; j++) {
            points[i].components[j] = (i+3)*(j+2)%10;
        }
    }

   return points; 
}


void print_points(Point *points, int size, int d) {
    int i;
    for(i=0; i<size; i++) {
        print_point(points[i], d);
    }
}


void print_points_in_range(Point *points, int size, int d, int low, int high) {
    int i;
    for(i=low; i<high; i++) {
        print_point(points[i], d);
    }
}


void print_point(Point p, int dimensions) {
    printf("(");
    int i;
    for (i=0; i<dimensions; i++) {
        printf("%d, ", p.components[i]);
    }
    printf(")\n");
}


void swap(Point *points, int first, int second) {
    Point temp = points[second];
    points[second] = points[first];
    points[first] = temp;
}


//[low, high)
void sort(Point *points, int low, int high, int dimension) {
    if (high-low <= 1) {
        return;
    }
    int pivot = (rand() % (high-low)) + low;    
    int pivot_value = points[pivot].components[dimension-1];
    swap(points, pivot, high-1);

    int i, small_sorted;
    for (i = low, small_sorted = 0; i < high-1; i++) {
        if (points[i].components[dimension-1] < pivot_value) {
            swap(points, i, small_sorted+low);
            small_sorted++;
        }
    }
    swap(points, small_sorted+low, high-1);


    sort(points, low, low+small_sorted, dimension);
    sort(points, low+small_sorted+1, high, dimension);

}


//returns 0 for failure, 1 for success
int check_sorted(Point *points, int size, int dimension) {
    int i, prev;
    prev = points[0].components[dimension];
    for (i=1; i<size; i++) {
        int curr = points[i].components[dimension];
        if (curr < prev) {
            return 0;
        }

        prev = curr;
    }

    return 1;
}


void test_sort(void) {
    int size = 1000000;
    int dimensions = 5;
    Point *points = generate_random(size, dimensions);

    int dimension;
    for (dimension = 0; dimensions < dimensions; dimension++) {
        sort(points, 0, size, dimension);
        if (!check_sorted(points, size, dimension)) {
            perror("Sorting Failure!\n"); 
            return;
        }
    }

    printf("Success!\n");
}


void insert(Point *point) {

}


void print_tree(Node *root, int dimensions) {
    if (root == NULL) {
        return;
    }
    print_tree(root->left_child, dimensions);
    print_point(root->point, dimensions);
    print_tree(root->right_child, dimensions);
}


//points are still sorted 
Node *build_subtree(Point *points, int low, int high, int dimension, int total_dimensions) {
    if (high-low == 0) {
        return NULL;
    } 

    //printf("High: %d, Low: %d\n", high, low);
    
    Node *new_node = malloc(sizeof(Node));
    int pos = ((high-low) / 2) + low;
    new_node->point = points[pos];
    new_node->left_child = build_subtree(points, low, pos, dimension, total_dimensions);
    new_node->right_child = build_subtree(points, pos+1, high, dimension, total_dimensions);

    if (dimension < total_dimensions) {
        //construct d-1 range tree
        //printf("Constructing %d Tree\n", dimension+1);
        int new_size = high - low;
        Point *new_points = malloc(sizeof(Point)*new_size);
        int i;
        for(i = 0; i < new_size; i++) {
            new_points[i] = points[i+low];
        }

        new_node->rt = build_range_tree(new_points, new_size, dimension+1, total_dimensions);

        free(new_points);
    } else {
        new_node->rt = NULL;
    }


    return new_node;
}


//[low, high)
//sorts points then builds balanced tree based on sorted array
RangeTree *build_range_tree(Point *points, int size, int dimension, int total_dimensions) {
    //First Dimension
    sort(points, 0, size, dimension);
    //print_points(points, size, total_dimensions);
    //initialize rt
    RangeTree *rt = malloc(sizeof(RangeTree));
    rt->size = size;
    rt->dimensions = total_dimensions;
    rt->root = build_subtree(points, 0, size, dimension, total_dimensions);

    //printf("Final Root:\n");
    //print_point(rt->root->point, dimensions);
    //root is middle element

    return rt;
}

int check_subtree_ordering(Node *root, int current_dimension, int min, int max) {
    if (root == NULL) { 
        return 1;
    }

    int root_value = root->point.components[current_dimension-1];
    if (root_value < min || root_value > max) {
        perror("Out of Order!");
        return 0;
    }

    check_subtree_ordering(root->left_child, current_dimension, min, root_value);
    check_subtree_ordering(root->right_child, current_dimension, root_value, max);

    return 1;
}


int check_range_subtrees(Node *root, int current_dimension, int total_dimensions) {
    if (root == NULL) {
        return 1;
    }

    if (current_dimension < total_dimensions) {
        if (!check_range_tree_ordering(root->rt->root, current_dimension+1, total_dimensions) ||
            !check_range_subtrees(root->left_child, current_dimension, total_dimensions) ||
            !check_range_subtrees(root->right_child, current_dimension, total_dimensions)) {
        
            return 0;
        }
    }

    return 1;
}


int check_range_tree_ordering(Node *root, int current_dimension, int total_dimensions) {
    //check ordering of subtree
    if(!check_subtree_ordering(root, current_dimension, 0, INT_MAX)) {
        return 0;
    }

    if(!check_range_subtrees(root, current_dimension, total_dimensions)) {
        return 0;
    }

    return 1;
}


void test_range_tree_construction() {
    int size = 1000;
    int dimensions = 5;
    Point *points = generate_random(size, dimensions);

    RangeTree *rt = build_range_tree(points, size, 1, dimensions);

    return;

    if(check_range_tree_ordering(rt->root, 1, dimensions)) {
        printf("Success!\n");
    } else {
        perror("Error Building Range Tree\n");
    }
}


int main(void) {

    /*
    int size = 11;
    int dimensions = 3;
    Point *points = generate_random(size, dimensions);


    printf("Building Tree\n");

    RangeTree *rt = build_range_tree(points, size, 1, dimensions);
    */

    test_range_tree_construction();




    
    return 0;
}
