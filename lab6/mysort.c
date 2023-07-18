void mysort ( long* numbers, int n, int low, int high)
{
   if ( n != -1) mysort(numbers, -1, 0, n-1);
   else
   {
      if (low < high) 
      {
         long pivot = numbers[high];
         long tmp;
         int i = low - 1;
         
         for (int j = low; j <= high - 1; j++) {
            if (numbers[j] < pivot) {
               i++;
               tmp = numbers[i];
               numbers[i] = numbers[j];
               numbers[j] = tmp;
            }
         }
         tmp = numbers[i + 1];
         numbers[i + 1] = numbers[high];
         numbers[high] = tmp;
         
         mysort(numbers, -1, low, i);
         mysort(numbers, -1, i + 2, high);
      }
   }   
}
