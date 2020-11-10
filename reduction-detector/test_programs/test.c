int main() {
	int array[5] = {0, 1, 2, 3, 4};
	int sum = 0, x = 0;
	for(int i = 0; i < 5; i++){
         	sum = sum + array[i];//sim//done
	        sum = sum - array[i];//sim//done
	        sum = array[i] + sum;//sim//done
         }
}
