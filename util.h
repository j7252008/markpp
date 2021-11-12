int mjd(int day, int month, int year, int hour)
{
    int a, b;
    if (month <= 2) {
        month = month + 12;
        year = year - 1;
    }
    a = 10000.0 * year + 100.0 * month + day;
    if (a <= 15821004.1) {
        b = -2 * ((year + 4716) / 4) - 1179;
    }
    else {
        b = (year / 400) - (year / 100) + (year / 4);
    }
    a = 365.0 * year - 679004.0;
    return (a + b + (30.6001 * (month + 1)) + day + hour / 24.0);
}

double ipart(double x)
{
    return x > 0 ? floor(x) : ceil(x);
}

std::vector<double> quad(double ym, double yz, double yp)
{
    double nz, a, b, c, dis, dx, xe, ye, z1, z2;

    std::vector<double> quadout(5);

    nz = 0;
    a = 0.5 * (ym + yp) - yz;
    b = 0.5 * (yp - ym);
    c = yz;
    xe = -b / (2 * a);
    ye = (a * xe + b) * xe + c;
    dis = b * b - 4.0 * a * c;
    if (dis > 0) {
        dx = 0.5 * sqrt(dis) / abs(a);
        z1 = xe - dx;
        z2 = xe + dx;
        if (abs(z1) <= 1.0)
            nz += 1;
        if (abs(z2) <= 1.0)
            nz += 1;
        if (z1 < -1.0)
            z1 = z2;
    }
    quadout[0] = nz;
    quadout[1] = z1;
    quadout[2] = z2;
    quadout[3] = xe;
    quadout[4] = ye;
    return quadout;
}

double range(double x)
{
    double a, b;
    b = x / 360;
    a = 360 * (b - ipart(b));
    if (a < 0) {
        a = a + 360;
    }
    return a;
}

double frac(double x)
{
    double a = x - int(x);
    if (a < 0)
        a += 1;
    return a;
}

double lmst(double mjday, double glong)
{
    double lst, t, d;
    d = mjday - 51544.5;
    t = d / 36525.0;
    lst = range(280.46061837 + 360.98564736629 * d + 0.000387933 * t * t - t * t * t / 38710000);
    return (lst / 15.0 + glong / 15);
}

std::vector<double> minisun(double t)
{
    double p2 = 6.283185307, coseps = 0.91748, sineps = 0.39778;
    double L, M, DL, SL, X, Y, Z, RHO, ra, dec;

    std::vector<double> suneq(2);

    M = p2 * frac(0.993133 + 99.997361 * t);
    DL = 6893.0 * sin(M) + 72.0 * sin(2 * M);
    L = p2 * frac(0.7859453 + M / p2 + (6191.2 * t + DL) / 1296000);
    SL = sin(L);
    X = cos(L);
    Y = coseps * SL;
    Z = sineps * SL;
    RHO = sqrt(1 - Z * Z);
    dec = (360.0 / p2) * atan(Z / RHO);
    ra = (48.0 / p2) * atan(Y / (X + RHO));
    if (ra < 0)
        ra += 24;
    suneq[0] = dec;
    suneq[1] = ra;
    return suneq;
}

double sin_alt(int iobj, double mjd0, int hour, double glong, double cglat, double sglat)
{
    double mjday, t, ra, dec, tau, salt, rads = 0.0174532925;
    mjday = mjd0 + hour / 24.0;
    t = (mjday - 51544.5) / 36525.0;

    std::vector<double> objpos = minisun(t);
    ra = objpos[1];
    dec = objpos[0];
    tau = 15.0 * (lmst(mjday, glong) - ra);
    salt = sglat * sin(rads * dec) + cglat * cos(rads * dec) * cos(rads * tau);
    return salt;
}

void Cal(int mjday, int tz, double glong, double glat)
{
    double      date, ym, yz, above, utrise, utset;
    double      yp, nz, hour, z1, z2, rads = 0.0174532925;
    std::string always_up = "日不落";
    std::string always_down = "日不出";
    std::string outstring = "";
    // var resobj = {};

    double sinho = sin(rads * -0.833);
    double sglat = sin(rads * glat);
    double cglat = cos(rads * glat);
    date = mjday - tz / 24.0;

    bool rise = false;
    bool sett = false;
    above = false;
    hour = 1.0;
    ym = sin_alt(2, date, hour - 1.0, glong, cglat, sglat) - sinho;
    if (ym > 0.0)
        above = true;
    while (hour < 25 && (!sett || !rise)) {
        yz = sin_alt(2, date, hour, glong, cglat, sglat) - sinho;
        yp = sin_alt(2, date, hour + 1.0, glong, cglat, sglat) - sinho;
        std::vector<double> quadout = quad(ym, yz, yp);
        nz = quadout[0];
        z1 = quadout[1];
        z2 = quadout[2];
        double xe = quadout[3];
        double ye = quadout[4];

        if (nz == 1) {
            if (ym < 0.0) {
                utrise = hour + z1;
                rise = true;
            }
            else {
                utset = hour + z1;
                sett = true;
            }
        }

        if (nz == 2) {
            if (ye < 0.0) {
                utrise = hour + z2;
                utset = hour + z1;
            }
            else {
                utrise = hour + z1;
                utset = hour + z2;
            }
        }

        ym = yp;
        hour += 2.0;
    }
}
