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
    int subtree_min;
    int subtree_max;
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


int max(int first, int second) {
    return first > second ? first : second;
}

int min(int first, int second) {
    return first < second ? first : second;
}

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
            points[i].components[j] = i+j;
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
            printf("Sorting Failure!\n"); 
            return;
        }
    }

    printf("Success!\n");
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

    //subtract 1 to ensure left wins ties
    int pos = ((high-low) / 2) + low - ((high-low+1) % 2);

    //assign point (TODO: Can we switch this to a single int?)
    new_node->point = points[pos];

    //assign children
    if (high-low > 1) {
        new_node->left_child = build_subtree(points, low, pos+1, dimension, total_dimensions);
        new_node->right_child = build_subtree(points, pos+1, high, dimension, total_dimensions);
    } else {
        new_node->left_child = NULL;
        new_node->right_child = NULL;
    }


    new_node->subtree_min = (new_node->left_child == NULL ? new_node->point.components[dimension-1] :
                                new_node->left_child->subtree_min);

    new_node->subtree_max = (new_node->right_child == NULL ? new_node->point.components[dimension-1] :
                                new_node->right_child->subtree_max);


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


//frees the component array of each point then the point array
void free_points(Point *points, int size) {
    int i;
    for(i=0; i<size; i++) {
        free(points[i].components);
    }

    free(points);
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
        printf("Out of Order!\n");
        return 0;
    }

    if (!check_subtree_ordering(root->left_child, current_dimension, min, root_value) ||
        !check_subtree_ordering(root->right_child, current_dimension, root_value, max)) {
        
        return 0;
    }

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
    int size = 8;
    int dimensions = 3;
    Point *points = generate_random(size, dimensions);
    print_points(points, size, dimensions);

    RangeTree *rt = build_range_tree(points, size, 1, dimensions);


    if(check_range_tree_ordering(rt->root, 1, dimensions)) {
        printf("Success!\n");
    } else {
        printf("Error Building Range Tree\n");
    }

    printf("%d\n", rt->root->subtree_min);
    printf("%d\n", rt->root->subtree_max);

    print_tree(rt->root, dimensions);
}


RangeTree *find_rts_to_query(RangeTree *rt, int lower_bound, int uppoer_bound) {

    //if subtree->max < lower_bound, return 0
    //if subtree->min > lower_bound and sub_tree->max < upper_bound, return sub_tree
    //if left_subtree->
}


Node *find_split_node(Node *root, int lower_bound, int upper_bound, int dimension) {
    if (root == NULL) {
        return NULL;
    }

    int current_value = root->point.components[dimension-1]; 

    if (current_value < lower_bound) {
        return find_split_node(root->right_child, lower_bound, upper_bound, dimension);
    }
    if (current_value >= upper_bound) {
        return find_split_node(root->left_child, lower_bound, upper_bound, dimension);
    }
    
    return root;
}


//Takes two NULL-TERMINATED arrays of pointers to rts and combines them
//Returns a NULL-TERMINATED array!!
RangeTree **combine_rt_lists(RangeTree **rt_list_1, RangeTree **rt_list_2) {

    int size1 = 0, size2 = 0;
    RangeTree *temp;
    temp = rt_list_1[size1];
    if (rt_list_1 != NULL) {
        while((temp = rt_list_1[size1]) && temp != NULL) {
            printf("Here1\n");
            size1++;
        }
    }
    
    if (rt_list_2 != NULL) {
        while((temp = rt_list_2[size2]) && temp != NULL) {
            size2++;
        }
    }

    RangeTree **combined_list = calloc(size1+size2+1, sizeof(RangeTree*));

    printf("Creating combined List\n");
    int i;
    for(i=0; i < size1; i++) {
        combined_list[i] = rt_list_1[i];
    }
    for(i=0; i < size2; i++) {
        combined_list[i+size1] = rt_list_2[i];
    }

    printf("Freeing\n");
    rt_list_1 == NULL ? : free(rt_list_1);
    rt_list_2 == NULL ? : free(rt_list_2);

    //TODO: Redundant
    combined_list[size1+size2] = NULL;

    printf("Size: %d\n", size1+size2);

    return combined_list;
}


RangeTree **get_lower_bound_range_trees(Node *root, int lower_bound) {
    printf("HERE\n");
    if (root == NULL) {
        return NULL;
    }

    printf("Min: %d\n", root->subtree_min);

    if (root->subtree_min >= lower_bound) {
        printf("Returning LEFT TREE\n");
        RangeTree **new_list = calloc(2, sizeof(RangeTree*));
        new_list[0] = root->rt;
        return new_list;
    }

    if (root->right_child != NULL && root->right_child->subtree_min >= lower_bound) {
        //add entire subtree and step left        
        RangeTree **new_list = calloc(2, sizeof(RangeTree*));
        new_list[0] = root->right_child->rt;
        RangeTree **left_list = get_lower_bound_range_trees(root->left_child, lower_bound);

        return combine_rt_lists(new_list, left_list);
        
    } else {
        //step right
        return get_lower_bound_range_trees(root->right_child, lower_bound);
    }
}


RangeTree **get_upper_bound_range_trees(Node *root, int upper_bound) {
    if (root == NULL) {
        return NULL;
    }

    printf("Max: %d\n", root->subtree_max);
    if (root->subtree_max <= upper_bound) {
        printf("Returning RIGHT Tree\n");
        RangeTree **new_list = calloc(2, sizeof(RangeTree*));
        new_list[0] = root->rt;
        return new_list;
    }

    if (root->left_child != NULL && root->left_child->subtree_max <= upper_bound) {
        //add entire subtree and step left        
        RangeTree **new_list = calloc(2, sizeof(RangeTree*));
        new_list[0] = root->left_child->rt;
        RangeTree **right_list = get_upper_bound_range_trees(root->right_child, upper_bound);

        return combine_rt_lists(new_list, right_list);
        
    } else {
        //step right
        return get_upper_bound_range_trees(root->left_child, upper_bound);
    }
}


Point *query(RangeTree *rt, Point first_bound, Point second_bound, int dimension) {
    int total_dimensions = rt->dimensions;

    printf("getting bounds\n");

    Node *root = rt->root;
    printf("HERE\n");
    int lower_bound = min(first_bound.components[dimension-1], second_bound.components[dimension-1]);
    int upper_bound = max(first_bound.components[dimension-1], second_bound.components[dimension-1]);
    
    
    if(dimension < total_dimensions) {
        //list of range trees to search

        RangeTree **rts_to_query;
        
        printf("Finding split Node\n");
        Node *split_node = find_split_node(rt->root, lower_bound, upper_bound, dimension);
        if(split_node == NULL) {
            printf("No points in range\n");
            return NULL;
        }

        if ((split_node->subtree_min >= lower_bound) && (split_node->subtree_max <= upper_bound)) {
            printf("Just Split Node\n");
            rts_to_query = calloc(2, sizeof(RangeTree*));
            rts_to_query[0] = split_node->rt;
        } else {

            printf("Starting Search for lower_bound\n");
            RangeTree **lower_bound_trees = get_lower_bound_range_trees(split_node->left_child, lower_bound);

            printf("Starting Search for upper bound\n");
            RangeTree **upper_bound_trees = get_upper_bound_range_trees(split_node->right_child, upper_bound);

            printf("Combining final lists\n");
            rts_to_query = combine_rt_lists(lower_bound_trees, upper_bound_trees);
        }

        if (rts_to_query == NULL) {
            print("No Range Trees to query\n");
            return NULL;
        }

        //recursively query all range trees in above list
        
        Point *point_list = NULL;

        RangeTree *temp;
        int i=0;
        while((temp = rts_to_query[i]) && temp != NULL) {
            point_list = combine_point_lists(point_list, query(temp, first_bound, second_bound, dimension));
        }

        return point_list;

    } else {
        //last dimension, return points
        
    }

    
      

    //combine points returned by all recursive calls
    
}


Point *test_query(void) {

    int size = 10;
    int dimensions = 2;
    Point *points = generate_known(size, dimensions);

    print_points(points, size, dimensions);

    RangeTree *rt = build_range_tree(points, size, 1, dimensions);

    printf("Printing Tree\n");
    print_tree(rt->root, dimensions);

    Point *query_points = generate_random(2, dimensions);

    query_points[0].components[0] = 0;
    query_points[0].components[1] = 10;
    query_points[1].components[0] = 5;
    query_points[1].components[1] = 10;


    printf("Query Bounds:\n");
    print_point(query_points[0], dimensions);
    print_point(query_points[1], dimensions);

    query(rt, query_points[0], query_points[1], 1);

}


int main(void) {

    /*
    int size = 2000;
    int dimensions = 5;
    Point *points = generate_random(size, dimensions);


    RangeTree *rt = build_range_tree(points, size, 1, dimensions);
    free_points(points, size);
    */

    //test_range_tree_construction();
    test_query();



    
    return 0;
}
