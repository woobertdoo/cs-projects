struct Resource {
    const int classID;
    const int systemTotal;
    int numAllocated;
    int numRequested;
};

const int MAX_PER_RESOURCE_CLASS = 5;
const int NUM_RESOURCE_CLASSES = 10;
static Resource resources[10] = {
    {0, MAX_PER_RESOURCE_CLASS, 0, 0}, {1, MAX_PER_RESOURCE_CLASS, 0, 0},
    {2, MAX_PER_RESOURCE_CLASS, 0, 0}, {3, MAX_PER_RESOURCE_CLASS, 0, 0},
    {4, MAX_PER_RESOURCE_CLASS, 0, 0}, {5, MAX_PER_RESOURCE_CLASS, 0, 0},
    {6, MAX_PER_RESOURCE_CLASS, 0, 0}, {7, MAX_PER_RESOURCE_CLASS, 0, 0},
    {8, MAX_PER_RESOURCE_CLASS, 0, 0}, {9, MAX_PER_RESOURCE_CLASS, 0, 0}};
