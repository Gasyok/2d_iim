#include <init/init.h>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <sstream>

double f(double x, double y) {
    return y - 0.3 - tan(0.4) * x;
    // return (y - 0.5) * (y - 0.5) + (x - 0.5) * (x - 0.5) - 0.2 * 0.2;
}

void InitValues::PrintInit() {
    std::ostringstream fileinit;
    fileinit << "../bin/animation/init_out.bin";
    std::ofstream file(fileinit.str(), std::ios::binary);
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            float x_coord = i * h;
            float y_coord = j * h;
            float pressure_value = u[i][j](2);
            // Запись x, y и p(x, y) (каждый параметр - 4 байта, little-endian)
            file.write(reinterpret_cast<char*>(&x_coord), sizeof(x_coord));
            file.write(reinterpret_cast<char*>(&y_coord), sizeof(y_coord));
            file.write(reinterpret_cast<char*>(&pressure_value), sizeof(pressure_value));
        }
    }
    file.close();
}
// void InitValues::SetInitU(double x0, double y0, double A, double omega, double alpha) {
//     // x0 = -0.3;
//     // y0 = 0.3 + tan(0.4) * x0;
//     double phi = 0.4;
//     Matrix2d R;
//     R << cos(phi), -sin(phi),
//           sin(phi), cos(phi);
//
//     Vector2d a_initial (cos(alpha), sin(alpha));
//     Vector2d a_reflected (cos(alpha), -sin(alpha));
//     Vector2d a_transmited (sqrt(1 - (c_plus / c_minus) * (c_plus / c_minus) * sin(alpha) * sin(alpha)), c_plus / c_minus * sin(alpha));
//
//     double cos_alpha_transm = sqrt(1 - (c_plus / c_minus) * (c_plus / c_minus) * sin(alpha) * sin(alpha));
//
//     double A_r = (rho_plus * c_plus * cos(alpha) - rho_minus * c_minus * cos_alpha_transm) / (rho_plus * c_plus * cos(alpha) + rho_minus * c_minus * cos_alpha_transm);
//     double A_t = 1 + A_r;
//     // double A_t = (2 * rho_plus * c_plus * cos(alpha)) / (rho_plus * c_plus * cos(alpha) + rho_minus * c_minus * cos_alpha_transm);
//
//     a_initial = R * a_initial;
//     a_reflected = R * a_reflected;
//     a_transmited = R * a_transmited;
//
//     double omega_t = omega * cos(alpha) / cos_alpha_transm;
//
//     for (int i = 0; i < size_x; ++i) {
//         pressure.push_back(vector<double>());
//         velocity_x.push_back(vector<double>());
//         velocity_y.push_back(vector<double>());
//         for (int j = 0; j < size_y; ++j) {
//             double xi_initial = a_initial(0) * (coord_x[i] - x0) + a_initial(1) * (coord_y[j] - y0);
//             double xi_reflected = a_reflected(0) * (coord_x[i] - x0) + a_reflected(1) * (coord_y[j] - y0);
//             double xi_transmited = a_transmited(0) * (coord_x[i] - x0) + a_transmited(1) * (coord_y[j] - y0);
//             double pressure_initial, pressure_reflected, pressure_transmited;
//             if (xi_initial >= 0 && xi_initial <= 1 / omega) {
//                 pressure_initial = 0.5 * A * (1 - cos(2 * M_PI * omega * xi_initial));
//             }
//             else {
//                 pressure_initial = 0.0;
//             }
//             if (xi_reflected >= 0 && xi_reflected <= 1 / omega) {
//                 pressure_reflected = 0.5 * A_r * (1 - cos(2 * M_PI * omega * xi_reflected));
//             }
//             else {
//                 pressure_reflected = 0.0;
//             }
//             if (xi_transmited >= 0 && xi_transmited <= 1 / omega_t) {
//                 pressure_transmited = 0.5 * A_t * (1 - cos(2 * M_PI * omega_t * xi_transmited));
//             }
//             else {
//                 pressure_transmited = 0.0;
//             }
//
//             if (f(coord_x[i], coord_y[j]) <= 0) {
//                 pressure[i].push_back(pressure_reflected + pressure_initial);
//                 velocity_x[i].push_back((pressure_initial * a_initial(0)  + pressure_reflected * a_reflected(0)) / (rho_minus * c_minus));
//                 velocity_y[i].push_back((pressure_initial * a_initial(1)  + pressure_reflected * a_reflected(1)) / (rho_minus * c_minus));
//             }
//             else {
//                 pressure[i].push_back(pressure_transmited);
//                 velocity_x[i].push_back(pressure_transmited * a_transmited(0) / (rho_plus * c_plus));
//                 velocity_y[i].push_back(pressure_transmited * a_transmited(1) / (rho_plus * c_plus));
//             }
//         }
//     }
// }
void InitValues::SetInitRadU(double x0, double y0, double omega) {
    double _pressure;
    for (int i = 0; i < M; ++i) {
        u.push_back(vector<Vector3d>());
        for (int j = 0; j < M; ++j) {
            double r = sqrt((i * h - x0) * (i * h - x0) + (j * h - y0) * (j * h - y0));
            if (r >= omega) {
                _pressure = 0.0;
            }
            else {
                _pressure = pow(cos(0.5 * M_PI * r / omega), 2);
            }
            u[i].push_back(Vector3d(0, 0, _pressure));
        }
    }
}
InitValues::InitValues(int _M, double _x0, double _y0, double _A, double _omega, double _alpha)
    : M(_M), x0(_x0), y0(_y0), omega(_omega), alpha(_alpha), A(_A) {

    h = 2.0 / _M;

    rho_minus = 1;
    c_minus = 1;

    rho_plus = 1;
    c_plus = 1;

    k_minus = c_minus * c_minus * rho_minus;
    k_plus = c_plus * c_plus * rho_plus;

    cir_left = 0.4;
    cir_right = cir_left * c_plus / c_minus;

    tau = cir_left * h / c_minus;

    if (cir_right >= 1) {
        std::cout << "Error in CIR Right";
        exit(-1);
    }


    A_minus << 0, 0, 1 / rho_minus,
               0, 0, 0,
               k_minus, 0, 0;
    A_plus << 0, 0, 1 / rho_plus,
               0, 0, 0,
               k_plus, 0, 0;
    B_minus << 0, 0, 0,
               0, 0, 1 / rho_minus,
               0, k_minus, 0;
    B_plus << 0, 0, 0,
               0, 0, 1 / rho_plus,
               0, k_plus, 0;
    // SetInitU(x0, y0, A, omega, alpha);
    SetInitRadU(x0, y0, omega);
    PrintInit();
    std::cout << "TAU: " << tau << std::endl;
    std::cout << "CIR LEFT: " << c_minus * tau / h << std::endl;
    std::cout << "CIR RIGHT: " << c_plus * tau / h << std::endl;

}
