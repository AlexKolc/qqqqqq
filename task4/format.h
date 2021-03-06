//
// Created by Сашуля on 17.05.2016.
//

#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <cstddef>
#include <iomanip>
#include <cstdio>
#include <typeinfo>

using namespace std;

/**
 * Returns a std::string formatted with modified syntax
 *
 * @param   fmt
 * @param   args Arguments required by the format specifiers in the format string.
 * @throws  std::invalid_format Argument can not be converted to unknown format.
 * @throws  std::out_of_range
 * @return  std::string, formatted string that used format and args.
 */

template<typename... Args>
string format(const string &fmt, const Args &... args);

namespace Format {
    enum length_format {
        l,      // int or unsined int
        hh,     // long int or unsigned long int, but -> signed char or unsigned char
        h,      // int or unsigned int, but -> short int or unsigned short int
        ll,     // long long int or unsigned long long int
        j,      // intmax_t or uintmax_t
        z,      // size_t (size ~ signed type)
        t,      //ptrdiff_t (size ~ unsigned type)
        L,      // __int64 or unsigned __int64
        absent, // int or unsigned int
        error   // if unknown length specifier
    };

    struct format_type {
        bool is_positive = false,       // '+'
                is_negative = false,    // '-'
                is_space = false,       // ' '
                is_sharp = false,       // '#'
                is_zero = false,        // '0'
                is_upcase = false,      // flag for type register
                is_floating = false;    // floating point number type
        int width = 0, precision = -1;
        char type;
        enum length_format capacity = absent;
    };

    template<typename In, typename Out>
    typename enable_if<is_convertible<Out, In>::value, In>::type parse(Out force) {
        return (In) force;
    }

    template<typename In, typename Out>
    typename enable_if<!is_convertible<Out, In>::value, In>::type parse(Out force) {
        throw invalid_argument("Invalid argument type.");
    }

    string check_specifier(const string &fmt, unsigned &pos, bool flag);

    string implementation(const string &fmt, unsigned pos, unsigned outprint);

    string nullptr_exception(nullptr_t force);

    template<typename T>
    typename enable_if<!is_integral<T>::value && !is_convertible<T, string>::value &&
                       !is_pointer<T>::value, string>::type nullptr_exception(const T &force) {
        throw invalid_argument("Invalid argument type.");
    }

    template<typename T>
    typename enable_if<is_integral<T>::value, string>::type nullptr_exception(T force) {
        return to_string(force);
    }

    template<typename T, int pos>
    typename enable_if<!is_convertible<T *, string>::value, string>::type nullptr_exception(const T (&arg)[pos]) {
        string outcome = "[";
        for (int i = 0; i < pos - 1; i++)
            outcome += to_string(arg[i]) + ", ";
        outcome += to_string(arg[pos - 1]) + ']';
        return outcome;
    }

    template<typename T>
    typename enable_if<is_convertible<T, string>::value, string>::type nullptr_exception(const T &force) {
        return force;
    }

    template<typename T>
    typename enable_if<
            !is_array<T>::value && !is_convertible<T, string>::value &&
            is_pointer<T>::value, string>::type nullptr_exception(
            T &force) {
        string outcome = "";
        if (!force)
            outcome += "nullptr<" + (string) typeid(*force).name() + ">";
        else
            outcome += "ptr<" + (string) typeid(*force).name() + ">(" + format("%@", *force) + ")";
        return outcome;
    }

    template<typename T>
    typename enable_if<is_arithmetic<T>::value, string>::type outprint_num(format_type _fmt, T force) {
        if (!_fmt.is_floating) {
            if (_fmt.precision < 0)
                _fmt.precision = 1;
            else if (_fmt.is_zero)
                _fmt.is_zero = false;
        }

        string tmp = "%";
        tmp += ((_fmt.is_positive) ? "+" : "");
        tmp += ((_fmt.is_negative) ? "-" : "");
        tmp += ((_fmt.is_space) ? " " : "");
        tmp += ((_fmt.is_sharp) ? "#" : "");
        tmp += ((_fmt.is_zero) ? "0" : "");
        if (_fmt.precision >= 0) tmp += '.' + to_string(_fmt.precision > 1024 ? 1024 : _fmt.precision);
        char buf[2048];
        if (_fmt.is_floating) {
            if (_fmt.capacity == L) tmp += 'L';
            if (_fmt.capacity == l) tmp += 'l';
            tmp += _fmt.type;
        } else
            tmp += "j", tmp += _fmt.type;
        snprintf(buf, sizeof(buf), tmp.c_str(), force);
        string outcome = buf;
        if (_fmt.precision > 1024 && outcome.size() > 512) {
            if (_fmt.is_floating) {
                unsigned n = _fmt.precision - outcome.size() + outcome.find_first_of('.') + 1;
                for (unsigned i = 0; i < n; i++)
                    outcome += '0';
            } else {
                unsigned n = _fmt.precision - outcome.size();
                if (outcome[0] != '0') ++n;
                string extra = "";
                for (unsigned i = 0; i < n; i++)
                    extra += '0';
                outcome = outcome.substr(0, 2) + extra + outcome.substr(2);
            }
        }

        if ((unsigned) _fmt.width > outcome.size()) {
            if (_fmt.is_negative) {
                unsigned n = _fmt.width - outcome.size();
                for (unsigned i = 0; i < n; i++)
                    outcome += ' ';
            }
            else {
                if (_fmt.is_zero) {
                    if (!outcome.find_first_of("+- ")) {
                        unsigned n = _fmt.width - outcome.size();
                        string extra = "";
                        for (unsigned i = 0; i < n; i++)
                            extra += '0';
                        outcome = outcome[0] + extra + outcome.substr(1);
                    } else {
                        unsigned n = _fmt.width - outcome.size();
                        string extra = "";
                        for (unsigned i = 0; i < n; i++)
                            extra += '0';
                        outcome = extra + outcome;
                    }
                } else {
                    unsigned n = _fmt.width - outcome.size();
                    string extra = "";
                    for (unsigned i = 0; i < n; i++)
                        extra += ' ';
                    outcome = extra + outcome;
                }
            }
        }

        return outcome;
    }

    template<typename In, typename... Out>
    string implementation(const string &fmt, unsigned pos, unsigned outprint, const In &force, const Out &... args) {
        string outcome = check_specifier(fmt, pos, true);
        format_type _fmt;
        string tmp = "";
        string for_check = "-+ #0";
        while (pos < fmt.length() && for_check.find(fmt[pos]) != string::npos)
            switch (fmt[pos++]) {
                case '-':
                    _fmt.is_negative = true;
                    _fmt.is_zero = false;
                    break;
                case '+':
                    _fmt.is_positive = true;
                    _fmt.is_space = false;
                    break;
                case ' ':
                    _fmt.is_space = !_fmt.is_positive;
                    break;
                case '#':
                    _fmt.is_sharp = true;
                    break;
                case '0':
                    _fmt.is_zero = !_fmt.is_negative;
                    break;
            }

        if (pos < fmt.length() && fmt[pos] == '*') {
            _fmt.width = parse<int>(force);
            if (_fmt.width < 0) {
                _fmt.width *= -1;
                _fmt.is_negative = true;
                _fmt.is_zero = false;
            }
            tmp = "%";
            tmp += ((_fmt.is_positive) ? "+" : "");
            tmp += ((_fmt.is_negative) ? "-" : "");
            tmp += ((_fmt.is_space) ? " " : "");
            tmp += ((_fmt.is_sharp) ? "#" : "");
            tmp += ((_fmt.is_zero) ? "0" : "");
            tmp += to_string(_fmt.width);
            string extra = implementation(tmp + fmt.substr(pos + 1, string::npos), 0, outprint + outcome.length(),
                                          args...);
            return outcome + extra;

        } else {
            while (pos < fmt.length() && isdigit(fmt[pos]))
                tmp += fmt[pos++];
            if (!tmp.empty())
                _fmt.width = stoi(tmp), tmp.clear();
        }

        if (pos < fmt.length() - 1 && fmt[pos] == '.') {
            pos++;
            if (fmt[pos] == '*') {
                _fmt.precision = parse<int>(force);
                tmp = "%";
                tmp += ((_fmt.is_positive) ? "+" : "");
                tmp += ((_fmt.is_negative) ? "-" : "");
                tmp += ((_fmt.is_space) ? " " : "");
                tmp += ((_fmt.is_sharp) ? "#" : "");
                tmp += ((_fmt.is_zero) ? "0" : "");
                if (_fmt.width != 0) tmp += to_string(_fmt.width);
                tmp += '.' + to_string(_fmt.precision);
                string extra = implementation(tmp + fmt.substr(pos + 1, string::npos), 0, outprint + outcome.length(),
                                              args...);
                return outcome + extra;
            } else {
                if (fmt[pos] == '-')
                    _fmt.precision = -1, pos++;
                else
                    _fmt.precision = 1;
                while (pos < fmt.length() && isdigit(fmt[pos]))
                    tmp += fmt[pos++];
                if (!tmp.empty())
                    _fmt.precision *= stoi(tmp), tmp.clear();
                else
                    _fmt.precision = 0;
            }
        }
        for_check = "hljztL";
        while (pos < fmt.length() && for_check.find(fmt[pos]) != string::npos)
            switch (fmt[pos++]) {
                case 'h':
                    if (_fmt.capacity == h)
                        _fmt.capacity = hh;
                    else if (_fmt.capacity == absent)
                        _fmt.capacity = h;
                    else
                        _fmt.capacity = error;
                    break;
                case 'l':
                    if (_fmt.capacity == l)
                        _fmt.capacity = ll;
                    else if (_fmt.capacity == absent)
                        _fmt.capacity = l;
                    else
                        _fmt.capacity = error;
                    break;
                case 'j':
                    if (_fmt.capacity == absent)
                        _fmt.capacity = j;
                    else
                        _fmt.capacity = error;
                    break;
                case 'z':
                    if (_fmt.capacity == absent)
                        _fmt.capacity = z;
                    else
                        _fmt.capacity = error;
                    break;
                case 't':
                    if (_fmt.capacity == absent)
                        _fmt.capacity = t;
                    else
                        _fmt.capacity = error;
                    break;
                case 'L':
                    if (_fmt.capacity == absent)
                        _fmt.capacity = L;
                    else
                        _fmt.capacity = error;
                    break;
            }

        if (pos == fmt.length())
            throw invalid_argument("Uncertain end of format.");
        if (_fmt.capacity == error)
            throw invalid_argument("Unknown length specifier.");

        stringstream output;
        if (_fmt.is_positive) output << showpos;
        if (_fmt.is_negative) output << left;
        if (_fmt.width != 0) output.width(_fmt.width);
        if (_fmt.precision >= 0) output.precision(_fmt.precision);
        if (_fmt.is_sharp) output << showbase << showpoint;

        uintmax_t u;    // unsigned type
        intmax_t d;     // integer type
        double f;       // float type
        char null_p[8]; // null pointer

        /**
         * | d, i | decimal signed number       |   sizeof( int )   |
         * | o    | octal unsigned number       |   sizeof( int )   |
         * | u    | decimal unsigned number     |   sizeof( int )   |
         * | x, X | hexadecimal number in the   |   sizeof( int )   |
         *          appropriate register
         * | f, F | floating point number       |  sizeof( double ) |
         * | e, E | floating point number
         *          in exponential form
         * | g, G | floating point number
         * | a, A | floating point number in
         *          hexadecimal form
         * | c    | symbol that has a code
         * | s  S | string with '\0' symbol
         *          at the end
         * | p    | pointer
         * | n    | recording pointer as an
         *          argument
         */

        _fmt.type = fmt[pos++];
        switch (_fmt.type) {
            case 'd':
            case 'i':
                switch (_fmt.capacity) {
                    case hh:
                        d = parse<signed char>(force);
                        break;
                    case h:
                        d = parse<short int>(force);
                        break;
                    case l:
                        d = parse<long int>(force);
                        break;
                    case ll:
                        d = parse<long long int>(force);
                        break;
                    case j:
                        d = parse<intmax_t>(force);
                        break;
                    case z:
                        d = parse<size_t>(force);
                        break;
                    case t:
                        d = parse<ptrdiff_t>(force);
                        break;
                    case absent:
                        d = parse<int>(force);
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                outcome += outprint_num(_fmt, d);
                break;
            case 'X':
                _fmt.is_upcase = true;
            case 'x':
            case 'o':
            case 'u':
                switch (_fmt.capacity) {
                    case hh:
                        u = parse<unsigned char>(force);
                        break;
                    case h:
                        u = parse<unsigned short int>(force);
                        break;
                    case l:
                        u = parse<unsigned long int>(force);
                        break;
                    case ll:
                        u = parse<unsigned long long int>(force);
                        break;
                    case j:
                        u = parse<uintmax_t>(force);
                        break;
                    case z:
                        u = parse<size_t>(force);
                        break;
                    case t:
                        u = parse<ptrdiff_t>(force);
                        break;
                    case absent:
                        u = parse<unsigned int>(force);
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                outcome += outprint_num(_fmt, u);
                break;
            case 'E':
            case 'G':
            case 'A':
                _fmt.is_upcase = true;
            case 'e':
            case 'g':
            case 'a':
            case 'F':
            case 'f':
                _fmt.is_floating = true;
                switch (_fmt.capacity) {
                    case l:
                    case absent:
                        f = parse<double>(force);
                        break;
                    case L:
                        f = parse<long double>(force);
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                outcome += outprint_num(_fmt, f);
                break;
            case 'c':
                switch (_fmt.capacity) {
                    case l:
                        break;
                    case absent:
                        output << parse<unsigned char>(force);
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                outcome += output.str();
                break;
            case 's': {
                string str;
                switch (_fmt.capacity) {
                    case l:
                        break;
                    case absent:
                        str = parse<string>(force);
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                if (str.length() > (unsigned) _fmt.precision && _fmt.precision >= 0)
                    str = str.substr(0, _fmt.precision);
                output << str;
                outcome += output.str();
            }
                break;
            case 'p':
                if (_fmt.capacity != absent)
                    throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                output << setfill(_fmt.is_zero ? '0' : ' ');
                snprintf(null_p, 2, "%p", parse<void *>(force));
                if (null_p[0] != '(' && parse<void *>(force) != NULL && parse<void *>(force) != nullptr)
                    output << parse<void *>(force);
                else
                    output << "(nil)";
                outcome += output.str();
                break;
            case 'n':
                outprint += outcome.length();
                switch (_fmt.capacity) {
                    case hh:
                        *(parse<signed char *>(force)) = outprint;
                        break;
                    case h:
                        *(parse<short int *>(force)) = outprint;
                        break;
                    case l:
                        *(parse<long int *>(force)) = outprint;
                        break;
                    case ll:
                        *(parse<long long int *>(force)) = outprint;
                        break;
                    case j:
                        *(parse<intmax_t *>(force)) = outprint;
                        break;
                    case z:
                        *(parse<size_t *>(force)) = outprint;
                        break;
                    case t:
                        *(parse<ptrdiff_t *>(force)) = outprint;
                        break;
                    case absent:
                        *(parse<int *>(force)) = outprint;
                        break;
                    default:
                        throw invalid_argument("Unknown specifier. Uncertain capacity variable.");
                }
                break;
            case '@':
                outcome += nullptr_exception(force);
                break;
            default:
                throw invalid_argument("Unknown specifier.");
                break;
        }
        string extra = implementation(fmt, pos, outprint + outcome.length(), args...);
        return outcome + extra;
    }
}

template<typename... Args>
string format(const string &fmt, const Args &... args) {
    return Format::implementation(fmt, 0, 0, args...);
}

#endif //TASK4_FORMAT_H
