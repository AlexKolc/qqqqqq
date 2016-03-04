class rational
{
    private:
        int n;
        int d;
        static int gcd(int, int);
    public:
        rational(int);
        rational(int, int);
        int getNum() const;
        int getDenom() const;
        rational operator +(rational const & x) const;
        rational operator -(rational const & x) const;
        rational operator *(rational const & x) const;
        rational operator /(rational const & x) const;
};
