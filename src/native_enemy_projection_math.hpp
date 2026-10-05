#pragma once
#include <cmath>
#include <complex>
#include <stdexcept>

// Analytic projection operands used by the connected Native enemy runtime.
// Whole block2 final Source/Native validation is still pending.
// Onpu299 uses a shared phase34, step20, heading base58 +/-20*sin(phase),
// pitch base5C +20*sin(phase). Keep that coupling in E801C's XYZ projection.
namespace dora3::native_time::enemy_math {

// Integer-order Bessel coefficient for the reviewed modulation range <=pi rad (reviewed 90-degree heading and 60-degree pitch).
// A bounded power series avoids adding a platform-specific library dependency.
inline double integer_bessel(unsigned order,double modulation) {
    if(!std::isfinite(modulation)||std::abs(modulation)>std::acos(-1.)+1e-9)
        throw std::logic_error("Unreviewed enemy angular modulation amplitude");
    double term=1;
    const double half=modulation/2;
    for(unsigned n=1;n<=order;++n)term*=half/n;
    double result=term;
    for(unsigned n=1;n<=32;++n) {
        term*=-(half*half)/(double(n)*(order+n));
        result+=term;
        if(std::abs(term)<1e-20)break;
    }
    return result;
}

// Analytic continuation of the *discrete* kick-before-drift harmonic sum:
// sum_j (speed+acceleration*j) exp(i*(carrier+A*sin(phase+step*j))).
// Caller supplies the existing reviewed harmonic_sum. At h=1 the runtime
// must keep actual E801C proposal bits and actual Source float store order.
// This helper only evaluates intermediate Native ages within that period.
template<class HarmonicSum>
inline std::complex<double> modulated_sum(double carrier,double amplitude,
        double phase,double step,double age,double speed,double acceleration,
        HarmonicSum&& harmonic_sum) {
    if(age==0)return {};
    if(!std::isfinite(carrier)||!std::isfinite(phase)||!std::isfinite(step)||
       !std::isfinite(age)||age<0||age>1)
        throw std::logic_error("Enemy modulation outside one Source period");
    auto result=integer_bessel(0,amplitude)*harmonic_sum(carrier,0,age,speed,acceleration);
    for(unsigned k=1;k<=16;++k) {
        const double coefficient=integer_bessel(k,amplitude);
        // J_-k(A)=(-1)^k J_k(A); negative A is retained by the power series.
        result+=coefficient*harmonic_sum(carrier+k*phase,k*step,age,speed,acceleration);
        result+=(k%2?-coefficient:coefficient)*harmonic_sum(carrier-k*phase,-double(k)*step,age,speed,acceleration);
    }
    return result;
}

// Angles/amplitudes/phase/step are radians from their exact ROM-qualified
// equations. Both angular equations must refer to the SAME captured phase.
// Queries/reflections after E801C do not change the captured projected angles.
struct WaveProjection {
    double heading,pitch,heading_amplitude,pitch_amplitude,phase,step;
    double heading_step=0;
    template<class HarmonicSum>
    std::complex<double> sum(double heading_sign,double pitch_sign,double age,
            double speed,double acceleration,HarmonicSum&& harmonic_sum) const {
        return modulated_sum(heading_sign*heading+pitch_sign*pitch,
            heading_sign*heading_amplitude+pitch_sign*pitch_amplitude,
            phase,step,age,speed,acceleration,[&](double phi,double omega,double h,double v,double a){return harmonic_sum(phi,omega+heading_sign*heading_step,h,v,a);});
    }
    template<class HarmonicSum>
    double displacement(unsigned axis,double age,double speed,double acceleration,
            HarmonicSum&& harmonic_sum) const {
        if(axis>2)throw std::logic_error("Invalid enemy XYZ axis");
        if(axis==1)return -sum(0,1,age,speed,acceleration,harmonic_sum).imag();
        const auto result=(sum(1,1,age,speed,acceleration,harmonic_sum)+
                           sum(1,-1,age,speed,acceleration,harmonic_sum))/2.;
        return axis==0?result.imag():result.real();
    }
    double velocity(unsigned axis,double age,double speed,double acceleration) const {
        const double wave=std::sin(phase+step*age);
        const double yaw=heading+heading_step*age+heading_amplitude*wave;
        const double tilt=pitch+pitch_amplitude*wave;
        const double scalar=speed+acceleration*age;
        if(axis==1)return -scalar*std::sin(tilt);
        if(axis>2)throw std::logic_error("Invalid enemy XYZ axis");
        return scalar*std::cos(tilt)*(axis==0?std::sin(yaw):std::cos(yaw));
    }
};

// Soundorago2 yaw and pitch consume separate integer Source phases. The
// two Jacobi--Anger expansions preserve the product and discrete drift sum.
struct IndependentWaveProjection {
    double heading,pitch,heading_amplitude,pitch_amplitude,yaw_phase,yaw_step,pitch_phase,pitch_step;
    template<class HarmonicSum>
    std::complex<double> sum(double yaw_sign,double pitch_sign,double age,double speed,double acceleration,
            HarmonicSum&& harmonic_sum) const {
        return modulated_sum(yaw_sign*heading+pitch_sign*pitch,yaw_sign*heading_amplitude,
            yaw_phase,yaw_step,age,speed,acceleration,[&](double phi,double omega,double h,double v,double a){
                return modulated_sum(phi,pitch_sign*pitch_amplitude,pitch_phase,pitch_step,h,v,a,
                    [&](double inner_phi,double inner_omega,double inner_h,double inner_v,double inner_a){
                        return harmonic_sum(inner_phi,omega+inner_omega,inner_h,inner_v,inner_a);
                    });
            });
    }
    template<class HarmonicSum>
    double displacement(unsigned axis,double age,double speed,double acceleration,HarmonicSum&& harmonic_sum) const {
        if(axis==1)return -sum(0,1,age,speed,acceleration,harmonic_sum).imag();
        if(axis>2)throw std::logic_error("Invalid independent wave axis");
        const auto path=(sum(1,1,age,speed,acceleration,harmonic_sum)+sum(1,-1,age,speed,acceleration,harmonic_sum))/2.;
        return axis==0?path.imag():path.real();
    }
    double velocity(unsigned axis,double age,double speed,double acceleration) const {
        const double yaw=heading+heading_amplitude*std::sin(yaw_phase+yaw_step*age);
        const double tilt=pitch+pitch_amplitude*std::sin(pitch_phase+pitch_step*age),scalar=speed+acceleration*age;
        if(axis==1)return -scalar*std::sin(tilt);
        if(axis>2)throw std::logic_error("Invalid independent wave axis");
        return scalar*std::cos(tilt)*(axis==0?std::sin(yaw):std::cos(yaw));
    }
};

// Continuation of sum_k (speed+acceleration*k)*exp(z*k), k=1..h.
// Taylor moments around z=0 avoid subtracting nearly equal complex values.
inline std::complex<double> geometric_drift(std::complex<double> z,double age,
        double speed,double acceleration) {
    if(age==0)return {};
    if(std::abs(z)<1e-4) {
        const double h=age;
        const double moments[]={h,h*(h+1)/2,h*(h+1)*(2*h+1)/6,
            h*h*(h+1)*(h+1)/4,h*(h+1)*(2*h+1)*(3*h*h+3*h-1)/30,
            h*h*(h+1)*(h+1)*(2*h*h+2*h-1)/12};
        std::complex<double> term=1,sum=0;
        for(unsigned n=0;n<5;++n) {
            sum+=term*(speed*moments[n]+acceleration*moments[n+1]);
            term*=z/double(n+1);
        }
        return sum;
    }
    const auto q=std::exp(z),qh=std::exp(z*age),denominator=1.-q;
    const auto sum=q*(1.-qh)/denominator;
    const auto weighted=q*(1.-qh)/(denominator*denominator)-age*q*qh/denominator;
    return speed*sum+acceleration*weighted;
}

// Discrete sum with an arithmetic angular-rate kick. Euler--Maclaurin
// continues sum_j (v+a*j)*exp(i*(phi+w*j+b*j*(j-1)/2)). The Gaussian
// integral is evaluated by its convergent power series over h in [0,1];
// Bernoulli corrections preserve the discrete sum, not continuous-time ODE.
// At zero angular acceleration use the existing exact harmonic expression.
template<class HarmonicSum>
inline std::complex<double> quadratic_harmonic_sum(double phase,double step,double angular_acceleration,
        double age,double speed,double acceleration,HarmonicSum&& harmonic_sum) {
    if(angular_acceleration==0)return harmonic_sum(phase,step,age,speed,acceleration);
    if(age==0)return {};
    const std::complex<double> linear(0,step-angular_acceleration/2),quadratic(0,angular_acceleration/2);
    std::complex<double> previous=0,coefficient=1,integral=0;double power=age;
    for(unsigned n=0;n<96;++n) {
        integral+=coefficient*(speed*power/double(n+1)+acceleration*power*age/double(n+2));
        const auto next=(linear*coefficient+2.*quadratic*previous)/double(n+1);
        previous=coefficient;coefficient=next;power*=age;
    }
    constexpr double bernoulli[]={1./6,-1./30,1./42,-1./30,5./66,-691./2730,7./6,-3617./510,
        43867./798,-174611./330,854513./138,-236364091./2730,8553103./6,-23749461029./870,
        8615841276005./14322,-7709321041217./510};
    const auto corrections=[&](double x) {
        const auto local=linear+2.*quadratic*x;
        std::complex<double> older=0,prior=1,correction=0;
        for(unsigned n=1;n<=32;++n) {
            const auto next=(local*prior+2.*quadratic*older)/double(n);
            if(n%2)correction+=bernoulli[(n-1)/2]/double(n+1)*((speed+acceleration*x)*next+acceleration*prior);
            older=prior;prior=next;
        }
        return std::exp(linear*x+quadratic*x*x)*correction;
    };
    return std::polar(1.,phase)*(integral+((speed+acceleration*age)*std::exp(linear*age+quadratic*age*age)-speed)/2.+
        corrections(age)-corrections(0));
}

// Banyatto combines wrapped proportional yaw with independently sinusoidal
// pitch. Expand the actual discrete recurrence in exponential powers and
// pitch sidebands; all operands are captured before collision setters.
struct ExponentialHeadingWavePitchProjection {
    double target,heading_amplitude,retention,pitch,pitch_amplitude,phase,step;
    std::complex<double> sum(double yaw_sign,double pitch_sign,double age,
            double speed,double acceleration) const {
        if(age==0)return {};
        if(age<0||age>1||retention<=0||retention>=1||std::abs(heading_amplitude)>3.142)
            throw std::logic_error("Unreviewed mixed enemy angular projection");
        const double decay=std::log(retention);
        std::complex<double> result=0,coefficient=1;
        for(unsigned n=0;n<48;++n) {
            auto band=integer_bessel(0,pitch_sign*pitch_amplitude)*
                geometric_drift({decay*n,0},age,speed,acceleration);
            for(unsigned k=1;k<=16;++k) {
                const double bessel=integer_bessel(k,pitch_sign*pitch_amplitude);
                band+=bessel*std::polar(1.,k*phase)*geometric_drift({decay*n,k*step},age,speed,acceleration);
                band+=(k%2?-bessel:bessel)*std::polar(1.,-double(k)*phase)*
                    geometric_drift({decay*n,-double(k)*step},age,speed,acceleration);
            }
            result+=coefficient*band;
            coefficient*=std::complex<double>(0,yaw_sign*heading_amplitude)/double(n+1);
            if(std::abs(coefficient)<1e-22)break;
        }
        return std::polar(1.,yaw_sign*target+pitch_sign*pitch)*result;
    }
    double displacement(unsigned axis,double age,double speed,double acceleration) const {
        if(axis==1)return -sum(0,1,age,speed,acceleration).imag();
        if(axis>2)throw std::logic_error("Invalid mixed enemy axis");
        const auto path=(sum(1,1,age,speed,acceleration)+sum(1,-1,age,speed,acceleration))/2.;
        return axis==0?path.imag():path.real();
    }
    double velocity(unsigned axis,double age,double speed,double acceleration) const {
        const double yaw=target+heading_amplitude*std::pow(retention,age);
        const double tilt=pitch+pitch_amplitude*std::sin(phase+step*age),scalar=speed+acceleration*age;
        if(axis==1)return -scalar*std::sin(tilt);
        if(axis>2)throw std::logic_error("Invalid mixed enemy axis");
        return scalar*std::cos(tilt)*(axis==0?std::sin(yaw):std::cos(yaw));
    }
};

// Wrapped proportional pitch has an unwrapped target within +/-pi of the
// captured old angle. Expand exp(i*A*retention^k) in its convergent power
// series, preserving the discrete kick-before-drift projection recurrence.
struct ExponentialPitchProjection {
    double heading,heading_step,target,amplitude,retention;
    std::complex<double> sum(double yaw_sign,double pitch_sign,double age,
            double speed,double acceleration) const {
        if(age==0)return {};
        if(age<0||age>1||retention<=0||retention>=1||std::abs(amplitude)>3.142)
            throw std::logic_error("Unreviewed exponential enemy pitch");
        const double carrier=yaw_sign*heading+pitch_sign*target;
        const double omega=yaw_sign*heading_step;
        const double decay=std::log(retention);
        std::complex<double> coefficient=1,result=0;
        for(unsigned n=0;n<48;++n) {
            result+=coefficient*geometric_drift({decay*n,omega},age,speed,acceleration);
            coefficient*=std::complex<double>(0,pitch_sign*amplitude)/double(n+1);
            if(std::abs(coefficient)<1e-22)break;
        }
        return std::polar(1.,carrier)*result;
    }
    double displacement(unsigned axis,double age,double speed,double acceleration) const {
        if(axis==1)return -sum(0,1,age,speed,acceleration).imag();
        if(axis>2)throw std::logic_error("Invalid exponential enemy axis");
        const auto value=(sum(1,1,age,speed,acceleration)+sum(1,-1,age,speed,acceleration))/2.;
        return axis==0?value.imag():value.real();
    }
    double velocity(unsigned axis,double age,double speed,double acceleration) const {
        const double tilt=target+amplitude*std::pow(retention,age);
        const double yaw=heading+heading_step*age,scalar=speed+acceleration*age;
        if(axis==1)return -scalar*std::sin(tilt);
        if(axis>2)throw std::logic_error("Invalid exponential enemy axis");
        return scalar*std::cos(tilt)*(axis==0?std::sin(yaw):std::cos(yaw));
    }
};

}
