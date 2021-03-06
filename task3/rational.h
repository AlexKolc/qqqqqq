#ifndef TASK3_RATIONAL_H
#define TASK3_RATIONAL_H

class rational {
private:
    int n;
    int d;
    int gcd(int, int);
public:
    rational(int num, int denom);
    rational(int num);
    int getNum() const;
    int getDenom() const;
    rational operator +(rational const &x) const;
    rational operator -(rational const &x) const;
    rational operator *(rational const &x) const;
    rational operator /(rational const &x) const;

};


#endif //TASK3_RATIONAL_H
