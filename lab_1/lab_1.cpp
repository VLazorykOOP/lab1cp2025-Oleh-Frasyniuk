// func_regr.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <optional>

struct Pair { double x{}, y{}; };               
struct TextPair { std::string key; double val; }; 

template <typename T>
bool load_table(const std::string& fname,
                std::vector<T>&       out,
                char                  delim        = ';',
                bool                  skip_header  = false)
{
    
    std::ifstream f(fname);
    if (!f) { std::cerr << "Файл не відкрився: " << fname << '\n'; return false; }

    std::string line;
    if (skip_header) std::getline(f, line);      

    while (std::getline(f, line))
    {
        if (line.empty()) continue;
        std::replace(line.begin(), line.end(), ',', '.'); // «,» → «.»

        std::stringstream ss(line);

        if constexpr (std::is_same_v<T, Pair>)
        {
            Pair p;
            char sep;                             // очікуємо «;»
            if (ss >> p.x >> sep >> p.y) out.push_back(p);
        }
        else                                      // TextPair
        {
            TextPair tp;
            char sep;
            if (std::getline(ss, tp.key, delim))
{
    std::string val_str;
    if (std::getline(ss, val_str))
    {
        try {
            tp.val = std::stod(val_str);
            out.push_back(tp);
        }
        catch (...) {
            // ігноруємо помилки
        }
    }
}

        }
    }

    /* ---- Правильне сортування залежно від типу ---- */
    if constexpr (std::is_same_v<T, Pair>)
    {
        std::sort(out.begin(), out.end(),
                  [](const Pair& a, const Pair& b) { return a.x < b.x; });
    }
    else  // TextPair
    {
        std::sort(out.begin(), out.end(),
                  [](const TextPair& a, const TextPair& b) { return a.val < b.val; });
    }

    return true;
}


std::optional<double> interpolate(const std::vector<Pair>& v, double x)
{
    if (v.empty()) return std::nullopt;

    auto it = std::find_if(v.begin(), v.end(),
                           [x](const Pair& p){ return p.x == x; });
    if (it != v.end()) return it->y;

    // за межами діапазону
    if (x < v.front().x || x > v.back().x) return std::nullopt;

    // знайти сусідні точки
    auto hi = std::upper_bound(v.begin(), v.end(), x,
                               [](double val, const Pair& p){ return val < p.x; });
    auto lo = hi - 1;

    double x0 = lo->x, y0 = lo->y;
    double x1 = hi->x, y1 = hi->y;

    double y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
    return y;
}

// пошук за ключем у таблиці 6
std::optional<double> find_text(const std::vector<TextPair>& v,
                                const std::string& key)
{
    auto it = std::find_if(v.begin(), v.end(),
                           [&key](const TextPair& p){ return p.key == key; });
    if (it != v.end()) return it->val;
    return std::nullopt;
}

// сама цільова функція (замість + можна вставити іншу формулу)
double func_regr(double r, double m, double k)
{
    return r + m + k;
}

// ─── main ─────────────────────────────────────────────────────────────────────
int main()
{
    std::vector<Pair>  tbl4, tbl5;
    std::vector<TextPair> tbl6;

    if (!load_table("dat1.dat", tbl4) ||
        !load_table("dat2.dat", tbl5) ||
        !load_table("dat3.dat", tbl6))
    {
        std::cerr << "Не вдалося завантажити всі таблиці.\n";
        return 1;
    }
    
    double x{}, y{}, z{}; std::string text;
    std::cout << "Введіть x y z text: ";
    if (!(std::cin >> x >> y >> z >> text))
    {
        std::cerr << "Помилка вводу.\n"; return 1;
    }

    auto r_opt = interpolate(tbl4, x);
    auto m_opt = interpolate(tbl5, y);
    auto k_opt = find_text(tbl6, text);

    if (!r_opt) { std::cerr << "x поза діапазоном таблиці 4.\n"; return 1; }
    if (!m_opt) { std::cerr << "y поза діапазоном таблиці 5.\n"; return 1; }
    if (!k_opt) { std::cerr << "text не знайдено у таблиці 6.\n"; return 1; }

    double result = func_regr(*r_opt, *m_opt, *k_opt);

    std::cout << std::fixed << std::setprecision(4)
              << "func_regr(" << *r_opt << ", " << *m_opt << ", "
              << *k_opt << ") = " << result << '\n';
    return 0;
}
