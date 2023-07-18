void mysort ( long* numbers, int n)
{
    int c,d;
    long swap;
    for (c = 0 ; c < n - 1; c++)
    {
        for (d = c+1 ; d < n; d++)
        {
            if (numbers[c] > numbers[d])
            {
                swap       = numbers[c];
                numbers[c]   = numbers[d];
                numbers[d] = swap;
            }
        }
    }
}

