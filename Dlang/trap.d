module trap;

import std.range;
import std.stdio;
import core.time;
import std.math;
import std.conv;

immutable int NSTEPS = 2000;
immutable int P_START = 0;
immutable int P_END = 5;

void main() {
    double step_size;
    double area;
    double p_current = P_START;
    double f_result_low, f_result_high;

    step_size = cast(double)(P_END - P_START) / NSTEPS;

    writeln(step_size);
    p_current = P_START;
    area = 0.0;

    MonoTime before = MonoTime.currTime;

    foreach(i; NSTEPS.iota) {

        p_current = i * step_size;

        f_result_low =  polynomial(p_current);
        f_result_high = polynomial(p_current + step_size);

        area += (f_result_low + f_result_high) * step_size / 2;
    }

    MonoTime after = MonoTime.currTime;
    auto elapsed = after - before;
    writeln("\nInterval length: ", elapsed.total!"msecs", " msecs");

    writeln("Result: ", area, "\n");
}

double polynomial(double x) {

    double numerator = pow(x, 2);

    double temp_poly = -4 * pow(x, 3) + 2 * pow(x, 4);

    double temp_poly_2 = pow(temp_poly, 4);

    double temp_poly_3 = pow(x, 3) + 2 * pow(x, 2) * temp_poly_2;

    double denominator = sqrt(temp_poly_3);

    double y = 0;
    if(denominator != 0) {
        y = numerator / denominator;
    }

    return y;
}