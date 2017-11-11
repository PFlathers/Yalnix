int main(int argc, char *argv[]) {
        int counter;
        int *counter_array;


        while (1) {
                counter_array = (int *) malloc(1000 * sizeof(int));
                TracePrintf(1, "1000 items allocated (should move brk)\n");

                for (counter = 0; counter < 1000; counter++) {
                        counter_array[counter] = counter * 2;
                }
        TracePrintf(1, "\tarray 0, 99, 999: %d, %d, %d\n", counter_array[0], counter_array[99], counter_array[999]);
        free(counter_array);
        Pause();

        }
    return 0;
}
