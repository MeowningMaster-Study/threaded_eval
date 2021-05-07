#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
using namespace std;

void f(double x, double &ret, mutex &m) {
    lock_guard<mutex> g(m); // блокируем доступ к ret для других потоков
    this_thread::sleep_for(chrono::seconds(5 + rand()%15)); // "засыпает" на 5-20 (случайно) секунд
    ret = x; //возвращает x
} // тут вызываеться деструктор lock_guard и он освобождает нашу переменную

void g(double x, double &ret, mutex &m) {
    lock_guard<mutex> g(m); // блокируем доступ к ret для других потоков
    ret = sqrt(x); // возвращает корень квадратный из x
} // тут вызываеться деструктор lock_guard и он освобождает нашу переменную

int main() {
    double x;
    cout << "Введіть x: ";
    cin >> x;

    srand((unsigned int)time(0)); // иниициализирует рандомизатор что бы использовать его в f
    double r1, r2; // переменные в которые мы будем возвращать результаты вычислений функций
    mutex m1, m2; // переменные-гаранты которые обеспечивают что только один поток будет менять выходные переменные одновременно
    thread t1(f, x, ref(r1), ref(m1)), t2(g, x, ref(r2), ref(m2)); // создает и запускает потоки для f и g

    this_thread::sleep_for(chrono::milliseconds(5)); // даем время потокам заблокировать всё что им нужно, похоже на костыль но работает

    auto start = chrono::steady_clock::now(); // засекаем начало
    bool l1 = false, l2 = false; // заблокированы ли переменные нашим основным потоком
    bool ask = true; // стоит ли спрашивать
    while (!(l1 && l2)) { // ждем доступа к переменным r1 и r2
        if (!l1) {
            l1 = m1.try_lock();
        }

        if (!l2) {
            l2 = m2.try_lock();
        }

        if (chrono::steady_clock::now() > start + 10s) { // если прошло 10 секунд
            if (ask) {

                cout << "Пройшло 10 секунд, продовжити обчислення? 1 - так, 2 - ні, 3 - більше не запитувати: ";
                int a;
                cin >> a;

                switch(a) {
                case 2: 
                    abort(); // завершаем программу и все дочерние потоки
                    break;
                case 3:
                    ask = false; // больше не спрашиваем
                    break;
                }
            }

            start = chrono::steady_clock::now();
        }
        
    }

    cout << r1 * r2 << endl;

    // ждём завершения потоков
    t1.join(); 
    t2.join();

    return 0;
}