#include "native_minigame_math.hpp"
#include "native_model_animation.hpp"
#include "recomp.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace dora3::native_time {
namespace {
bool field_pointer(std::uint32_t p) {
    return !(p&3)&&p>=0x80000000u&&std::uint64_t(p)+4<=0x80400000ull;
}
std::uint32_t bits(std::uint8_t* ram,std::uint32_t p) {
    auto* rdram=ram;
    return std::uint32_t(MEM_W(0,std::int32_t(p)));
}
float value(std::uint8_t* ram,std::uint32_t p) {
    return std::bit_cast<float>(bits(ram,p));
}
double wrapped(double v) {
    return v-360.0*std::floor(v/360.0);
}
}
struct MinigameEquations::State {
    enum class Kind : std::uint8_t {Constant,Load,Arithmetic,Advance,AffineAdvance,TimeOperand,Anchored,Reference};
    struct Node {
        Kind kind=Kind::Constant;
        Operation op=Operation::Add;
        Expression a=0,b=0;
        std::uint32_t address=0;
        double number=0;
    };
    struct Field {
        std::uint32_t address=0,source=0,published=0;
        Expression expression=0;
        FieldRole role=FieldRole::Scratch;
        Lifetime owner;
        double minimum=-std::numeric_limits<double>::infinity(),maximum=std::numeric_limits<double>::infinity();
        bool clamp=false;
    };
    std::vector<Node> nodes{{}};
    mutable std::array<std::vector<double>,5> integer_evaluations;
    std::vector<std::function<double(double)>> time_operands;
    std::vector<Field> fields;
    std::vector<std::uint32_t> changed_actors,changed_addresses;
    std::unordered_map<std::uint32_t,Expression> memory;
    std::unordered_map<std::uint32_t,std::uint32_t> memory_values;
    std::unordered_map<std::uint32_t,double> source_values;
    std::uint8_t* ram=nullptr;
    std::uint32_t rom=0;
    std::uint64_t scene=0;
    Nanoseconds source_time=0,shown_time=0;
    unsigned divider=3;
    bool open=false;
    Expression add(Node n) {
        nodes.push_back(n);return Expression(nodes.size()-1);
    }
    bool valid(const Field& f,std::uint8_t* image) const {
        if(image!=ram||f.owner.scene!=scene||!field_pointer(f.address))return false;
        if(!f.owner.actor)return true;
        const auto a=f.owner.actor;
        if(!field_pointer(a+0x98)||bits(image,a+0x94)!=f.owner.identity||!f.owner.identity)
            return false;
        if(bits(image,a+0xB0)!=f.owner.resources)return false;
        auto* rdram=image;
        if(f.owner.driver&&bits(image,a+0x98)!=f.owner.driver)return false;
        for(unsigned i=0;i<7;++i)
            if(bits(image,a+0x74+4*i)!=f.owner.components[i] ||
               MEM_BU(std::int32_t(0x6B+i),std::int32_t(a))!=f.owner.types[i] ||
               (f.owner.component_allocations[i]&&game_model_animation().lifetime_token(f.owner.components[i])!=f.owner.component_allocations[i]))return false;
        if(f.owner.model&&(!field_pointer(f.owner.model_slot)||bits(image,f.owner.model_slot)!=f.owner.model))return false;
        return !f.owner.allocation||game_model_animation().lifetime_token(f.owner.model)==f.owner.allocation;
    }
    bool dependent(Expression root,std::uint32_t address) const {
        if(!root||root>=nodes.size())return false;
        const auto& n=nodes[root];
        if(n.kind==Kind::Load)return n.address==address;
        if(n.kind==Kind::Anchored)return n.address==address||dependent(n.a,address);
        if(n.kind==Kind::Reference)return dependent(n.a,address);
        if(n.kind==Kind::Advance||n.kind==Kind::AffineAdvance)
            return n.address==address||dependent(n.a,address)||dependent(n.b,address);
        return n.kind==Kind::Arithmetic&&(dependent(n.a,address)||dependent(n.b,address));
    }
    // The Source update is a discrete recurrence, not a stored pose pair.
    // Polynomial steps use the fractional extension of their discrete sum.
    // In particular v'=v+a; x'=x+v' gives x(h)=x+v*h+a*h*(h+1)/2.
    // Multiplying the endpoint velocity by h would distort jumps at high FPS.
    int degree(Expression id,unsigned depth=0) const {
        if(depth>96)return -1;
        if(!id||id>=nodes.size())return -1;
        const auto& n=nodes[id];
        if(n.kind==Kind::Constant||n.kind==Kind::Load)return 0;
        if(n.kind==Kind::TimeOperand)return -1;
        if(n.kind==Kind::Advance) {
            if(n.op==Operation::Multiply)return -1;
            const int d=degree(n.a,depth+1);return d<0?-1:d+1;
        }
        if(n.kind==Kind::AffineAdvance)return -1;
        if(n.kind==Kind::Anchored||n.kind==Kind::Reference)return degree(n.a,depth+1);
        const int a=degree(n.a,depth+1),b=degree(n.b,depth+1);
        switch(n.op) {
        case Operation::Add:case Operation::Subtract:return a<0||b<0?-1:std::max(a,b);
        case Operation::Multiply:return a<0||b<0?-1:a+b;
        case Operation::Divide:return b==0?a:-1;
        case Operation::Negate:case Operation::RoundFloat:return a;
        default:return a==0&&(!n.b||b==0)?0:-1;
        }
    }
    double at(Expression id,double h) const {
        if(!open&&h>=0&&h<=4&&h==std::floor(h)) {
            auto& cached=integer_evaluations[unsigned(h)];
            if(cached.size()!=nodes.size())cached.assign(nodes.size(),std::numeric_limits<double>::quiet_NaN());
            return evaluate(id,h,cached);
        }
        std::vector<double> cache(nodes.size(),std::numeric_limits<double>::quiet_NaN());
        return evaluate(id,h,cache);
    }
    double integral(Expression id,double end) const {
        // Pure reviewed Source rates are evaluated at quadrature abscissae.
        // These are velocity/acceleration/curve operands, not stored poses.
        constexpr std::array<double,4> point{.1834346424956498,.5255324099163290,.7966664774136267,.9602898564975363};
        constexpr std::array<double,4> weight{.3626837833783620,.3137066458778873,.2223810344533745,.1012285362903763};
        double result=0;
        for(unsigned i=0;i<4;++i)result+=weight[i]*(at(id,end*(1-point[i])/2)+at(id,end*(1+point[i])/2));
        return result*end/2;
    }
    double sum(Expression id,double h) const {
        const int d=degree(id);
        if(d>=0&&d<=4) {
            std::array<double,5> difference{};
            for(int i=0;i<=d;++i)difference[i]=at(id,i);
            double result=0,choose=h+1;
            for(int k=0;k<=d;++k) {
                result+=difference[0]*(choose-(k==0?1.:0.));
                for(int j=0;j<d-k;++j)difference[j]=difference[j+1]-difference[j];
                choose*= (h-double(k))/double(k+2);
            }
            return result;
        }
        const auto& n=nodes[id];
        if(n.kind==Kind::Arithmetic) {
            if(n.op==Operation::Add)return sum(n.a,h)+sum(n.b,h);
            if(n.op==Operation::Subtract)return sum(n.a,h)-sum(n.b,h);
            if(n.op==Operation::Negate)return -sum(n.a,h);
            if(n.op==Operation::RoundFloat)return sum(n.a,h);
            if(n.op==Operation::Multiply) {
                if(degree(n.a)==0)return at(n.a,0)*sum(n.b,h);
                if(degree(n.b)==0)return at(n.b,0)*sum(n.a,h);
            }
            if(n.op==Operation::Divide&&degree(n.b)==0)return sum(n.a,h)/at(n.b,0);
            if((n.op==Operation::Sine||n.op==Operation::Cosine)&&degree(n.a)==1) {
                const double phase=at(n.a,0),step=at(n.a,1)-phase;
                if(std::abs(step)<1e-12)return h*at(id,0);
                const double divisor=std::sin(step/2);
                if(std::abs(divisor)<1e-12)return std::numeric_limits<double>::quiet_NaN();
                const double amplitude=std::sin(h*step/2)/divisor;
                const double angle=phase+(h+1)*step/2;
                return amplitude*(n.op==Operation::Sine?std::sin(angle):std::cos(angle));
            }
        }
        if(n.kind==Kind::Advance&&n.op==Operation::Multiply&&degree(n.a)==0) {
            const double factor=at(n.a,0);
            if(!(factor>0))return std::numeric_limits<double>::quiet_NaN();
            const auto found=source_values.find(n.address);
            const double base=found==source_values.end()?n.number:found->second;
            return factor==1?base*h:base*factor*std::expm1(h*std::log(factor))/(factor-1);
        }
        if(n.kind==Kind::AffineAdvance&&degree(n.a)==0&&degree(n.b)==0) {
            const double retention=at(n.a,0),forcing=at(n.b,0);
            if(!(retention>0))return std::numeric_limits<double>::quiet_NaN();
            const auto found=source_values.find(n.address);
            const double base=found==source_values.end()?n.number:found->second;
            if(retention==1)return base*h+forcing*h*(h+1)/2;
            const double equilibrium=forcing/(1-retention);
            const double geometric=retention*std::expm1(h*std::log(retention))/(retention-1);
            return equilibrium*h+(base-equilibrium)*geometric;
        }
        // The nonlinear rate integral is calibrated to the exact discrete
        // Source step D(1). It agrees at h=0/1, is independent of render slices,
        // and integrates the actual Source operand expression at fractional h.
        // For affine rates it reduces to the same h*(h+1)/2 Euler extension.
        // No previous/next ready pose enters this equation.
        return integral(id,h)+h*(at(id,1)-integral(id,1));
    }
    double evaluate(Expression id,double h,std::vector<double>& cache) const {
        if(!id||id>=nodes.size())return 0;
        if(cache[id]==std::numeric_limits<double>::infinity())return std::numeric_limits<double>::quiet_NaN();
        if(!std::isnan(cache[id]))return cache[id];
        cache[id]=std::numeric_limits<double>::infinity();
        const auto& n=nodes[id];double result=n.number;
        if(n.kind==Kind::Reference)result=evaluate(n.a,h,cache);
        else if(n.kind==Kind::Anchored) {
            const auto found=source_values.find(n.address);
            const double base=found==source_values.end()?n.number:found->second;
            const double from=at(n.a,0),to=evaluate(n.a,h,cache);
            result=base+(n.op==Operation::AngleWrap?std::remainder(to-from,360.):to-from);
        } else if(n.kind==Kind::TimeOperand)result=h==0?n.number:time_operands.at(n.address)(h);
        else if(n.kind==Kind::Load) {
            const auto at=source_values.find(n.address);
            result=at==source_values.end()?n.number:at->second;
        } else if(n.kind==Kind::Advance) {
            const auto at=source_values.find(n.address);
            const double base=at==source_values.end()?n.number:at->second;
            if(h==0)result=base;
            else if(n.op==Operation::Multiply) {
                const double factor=evaluate(n.a,0,cache);
                result=degree(n.a)==0&&factor>0?base*std::pow(factor,h):std::numeric_limits<double>::quiet_NaN();
            } else result=base+(n.op==Operation::Subtract?-1.:1.)*sum(n.a,h);
        } else if(n.kind==Kind::AffineAdvance) {
            const auto found=source_values.find(n.address);
            const double base=found==source_values.end()?n.number:found->second;
            const double retention=evaluate(n.a,0,cache),forcing=evaluate(n.b,0,cache);
            if(h==0)result=base;
            else if(degree(n.a)!=0||!(retention>0))result=std::numeric_limits<double>::quiet_NaN();
            else if(retention==1)result=base+sum(n.b,h);
            else if(degree(n.b)==0)result=base*std::pow(retention,h)+forcing*std::expm1(h*std::log(retention))/(retention-1);
            else {
                // A moving target contributes a weighted forcing rate. The
                // constant case is the exact fractional affine recurrence;
                // calibrating the variable rate preserves its next Source step.
                constexpr std::array<double,4> point{.1834346424956498,.5255324099163290,.7966664774136267,.9602898564975363};
                constexpr std::array<double,4> weight{.3626837833783620,.3137066458778873,.2223810344533745,.1012285362903763};
                const auto weighted=[&](double end) {
                    double total=0;
                    for(unsigned i=0;i<4;++i)for(double sign:{-1.,1.}) {
                        const double t=end*(1+sign*point[i])/2;
                        total+=weight[i]*std::pow(retention,end-t)*at(n.b,t);
                    }
                    return total*end/2*std::log(retention)/(retention-1);
                };
                const double factor=std::expm1(h*std::log(retention))/(retention-1);
                result=base*std::pow(retention,h)+weighted(h)+factor*(at(n.b,1)-weighted(1));
            }
        } else if(n.kind==Kind::Arithmetic) {
            const double a=evaluate(n.a,h,cache),b=evaluate(n.b,h,cache);
            switch(n.op) {
            case Operation::Add:result=a+b;break;
            case Operation::Subtract:result=a-b;break;
            case Operation::Multiply:result=a*b;break;
            case Operation::Divide:result=a/b;break;
            case Operation::Negate:result=-a;break;
            case Operation::Absolute:result=std::abs(a);break;
            case Operation::SquareRoot:result=std::sqrt(a);break;
            case Operation::Sine:result=std::sin(a);break;
            case Operation::Cosine:result=std::cos(a);break;
            case Operation::AngleWrap:result=wrapped(a);break;
            case Operation::AngleDelta:result=std::remainder(a,360.);break;
            case Operation::Heading:result=wrapped(std::atan2(a,b)*180./std::acos(-1.));break;
            case Operation::RoundFloat:result=float(a);break;
            }
        }
        cache[id]=result;return result;
    }
    // Decompose only proved self-dependent arithmetic into A*x+B. This
    // covers approach/damping without treating arbitrary pose stores as motion.
    // Whole-degree wrap and guest float rounding are handled at publication.
    std::optional<std::pair<Expression,Expression>> affine(Expression id,std::uint32_t address,bool angles=false) {
        const auto constant=[&](double v){return add({Kind::Constant,Operation::Add,0,0,0,v});};
        const auto operation=[&](Operation op,Expression a,Expression b=0){return add({Kind::Arithmetic,op,a,b,0,0});};
        if(!dependent(id,address))return std::pair{constant(0),id};
        const auto n=nodes[id];
        if(n.kind==Kind::Load&&n.address==address)return std::pair{constant(1),constant(0)};
        if(n.kind!=Kind::Arithmetic)return std::nullopt;
        if(n.op==Operation::RoundFloat||(angles&&n.op==Operation::AngleWrap))return affine(n.a,address,angles);
        if(n.op==Operation::Negate) {
            const auto a=affine(n.a,address,angles);if(!a)return std::nullopt;
            return std::pair{operation(Operation::Negate,a->first),operation(Operation::Negate,a->second)};
        }
        if(n.op==Operation::Add||n.op==Operation::Subtract) {
            const auto a=affine(n.a,address,angles),b=affine(n.b,address,angles);if(!a||!b)return std::nullopt;
            return std::pair{operation(n.op,a->first,b->first),operation(n.op,a->second,b->second)};
        }
        if(n.op==Operation::Multiply||n.op==Operation::Divide) {
            Expression dependent_part=n.a,independent_part=n.b;
            if(n.op==Operation::Multiply&&!dependent(n.a,address))std::swap(dependent_part,independent_part);
            if(dependent(independent_part,address))return std::nullopt;
            const auto a=affine(dependent_part,address,angles);if(!a)return std::nullopt;
            return std::pair{operation(n.op,a->first,independent_part),operation(n.op,a->second,independent_part)};
        }
        return std::nullopt;
    }
    Expression recurrence(std::uint32_t address,Expression expression,float previous,FieldRole role) {
        if(!expression||expression>=nodes.size()||role==FieldRole::Scratch||role==FieldRole::Derived)
            return expression;
        Expression arithmetic=expression;
        while((nodes[arithmetic].kind==Kind::Anchored&&nodes[arithmetic].address!=address)||
              (nodes[arithmetic].kind==Kind::Arithmetic&&
               (nodes[arithmetic].op==Operation::RoundFloat||
                (role==FieldRole::Angle&&nodes[arithmetic].op==Operation::AngleWrap))))
            arithmetic=nodes[arithmetic].a;
        const auto node=nodes[arithmetic];
        if(node.kind!=Kind::Arithmetic)return expression;
        if(node.op!=Operation::Add&&node.op!=Operation::Subtract&&node.op!=Operation::Multiply)
            return expression;
        const auto matches=[&](Expression id) {
            return id&&id<nodes.size()&&nodes[id].kind==Kind::Load&&nodes[id].address==address;
        };
        const auto previous_step=[&](Expression id)->std::optional<Node> {
            if(!id||id>=nodes.size())return std::nullopt;
            const auto& saved=nodes[id];
            if(saved.kind!=Kind::Anchored||saved.address!=address)return std::nullopt;
            const auto step=nodes[saved.a];
            if(step.kind==Kind::Advance||step.kind==Kind::AffineAdvance)return step;
            return std::nullopt;
        };
        const auto arithmetic_node=[&](Operation op,Expression a,Expression b){return add({Kind::Arithmetic,op,a,b,0,0});};
        Expression self=node.a,operand=node.b;
        if(node.op!=Operation::Subtract&&!previous_step(self)&&previous_step(node.b))std::swap(self,operand);
        if(const auto previous_node=previous_step(self);previous_node&&!dependent(operand,address)) {
            const auto one=add({Kind::Constant,Operation::Add,0,0,0,1});
            const auto zero=add({Kind::Constant,Operation::Add,0,0,0,0});
            auto retention=previous_node->kind==Kind::AffineAdvance?previous_node->a:
                previous_node->op==Operation::Multiply?previous_node->a:one;
            auto forcing=previous_node->kind==Kind::AffineAdvance?previous_node->b:
                previous_node->op==Operation::Multiply?zero:
                previous_node->op==Operation::Subtract?add({Kind::Arithmetic,Operation::Negate,previous_node->a,0,0,0}):previous_node->a;
            if(node.op==Operation::Multiply) {
                retention=arithmetic_node(Operation::Multiply,retention,operand);
                forcing=arithmetic_node(Operation::Multiply,forcing,operand);
            } else forcing=arithmetic_node(node.op,forcing,operand);
            if(degree(retention)==0&&at(retention,0)==1)
                return add({Kind::Advance,Operation::Add,forcing,0,address,previous});
            return add({Kind::AffineAdvance,Operation::Add,retention,forcing,address,previous});
        }
        Expression delta=0;
        if(matches(node.a)&&!dependent(node.b,address))delta=node.b;
        else if(node.op!=Operation::Subtract&&matches(node.b)&&!dependent(node.a,address))delta=node.a;
        if(delta)return add({Kind::Advance,node.op,delta,0,address,previous});
        if(const auto coefficients=affine(arithmetic,address,role==FieldRole::Angle)) {
            if(degree(coefficients->first)==0) {
                if(at(coefficients->first,0)==1&&!dependent(coefficients->second,address))
                    return add({Kind::Advance,Operation::Add,coefficients->second,0,address,previous});
                auto forcing=coefficients->second;
                if(role==FieldRole::Angle&&at(coefficients->first,0)==1)
                    forcing=add({Kind::Arithmetic,Operation::AngleDelta,forcing,0,0,0});
                return add({Kind::AffineAdvance,Operation::Add,coefficients->first,forcing,address,previous});
            }
        }
        return expression;
    }
    bool reaches(Expression root,Expression target) const {
        std::vector<Expression> todo{root};std::vector<bool> seen(nodes.size());
        while(!todo.empty()) {
            const auto id=todo.back();todo.pop_back();
            if(id==target)return true;
            if(!id||id>=nodes.size()||seen[id])continue;
            seen[id]=true;const auto& n=nodes[id];
            if(n.kind==Kind::Arithmetic||n.kind==Kind::Anchored||n.kind==Kind::Reference||n.kind==Kind::Advance||n.kind==Kind::AffineAdvance) {
                todo.push_back(n.a);todo.push_back(n.b);
            }
        }
        return false;
    }
    bool reads(Expression id,std::uint32_t address,unsigned depth=0) const {
        if(!id||id>=nodes.size()||depth>96)return depth>96;
        const auto n=nodes[id];
        if(n.kind==Kind::Load)return n.address==address;
        if(n.kind==Kind::Arithmetic||n.kind==Kind::Anchored||n.kind==Kind::Reference||
           n.kind==Kind::Advance||n.kind==Kind::AffineAdvance)
            return reads(n.a,address,depth+1)||reads(n.b,address,depth+1);
        return false;
    }
    void restore(std::uint8_t* image) {
        if(image!=ram)return;
        auto* rdram=image;
        for(auto& f:fields) {
            if(f.role==FieldRole::Scratch||!valid(f,image))continue;
            if(bits(image,f.address)==f.published)
                MEM_W(0,std::int32_t(f.address))=std::int32_t(f.source);
            else {
                // A real external command owns the new value. It must neither
                // be overwritten nor become another mathematical recurrence.
                f.role=FieldRole::Scratch;
            }
            f.published=bits(image,f.address);
        }
    }
};
MinigameEquations::MinigameEquations():state_(std::make_unique<State>()) {}
MinigameEquations::~MinigameEquations()=default;
void MinigameEquations::begin_source(std::uint8_t* ram,std::uint32_t module,
                                    std::uint64_t scene,Nanoseconds now,unsigned divider) {
    if(divider!=2&&divider!=3)return;
    auto& s=*state_;s.restore(ram);s.nodes={{}};for(auto& cached:s.integer_evaluations)cached.clear();s.time_operands.clear();s.fields.clear();s.memory.clear();s.memory_values.clear();s.source_values.clear();
    s.ram=ram;s.rom=module;s.scene=scene;s.source_time=s.shown_time=now;s.divider=divider;s.open=true;s.changed_actors.clear();s.changed_addresses.clear();
}
MinigameEquations::Expression MinigameEquations::constant(double number) {
    return state_->open?state_->add({State::Kind::Constant,Operation::Add,0,0,0,number}):0;
}
MinigameEquations::Expression MinigameEquations::time_operand(double number,std::function<double(double)> sample) {
    auto& s=*state_;if(!s.open||!sample)return 0;
    const auto index=std::uint32_t(s.time_operands.size());s.time_operands.push_back(std::move(sample));
    return s.add({State::Kind::TimeOperand,Operation::Add,0,0,index,number});
}
MinigameEquations::Expression MinigameEquations::load_float(std::uint32_t address,float number) {
    auto& s=*state_;if(!s.open)return 0;
    if(auto at=s.memory.find(address);at!=s.memory.end()&&s.memory_values.at(address)==std::bit_cast<std::uint32_t>(number))return at->second;
    return s.add({State::Kind::Load,Operation::Add,0,0,address,number});
}
MinigameEquations::Expression MinigameEquations::operation(Operation op,Expression a,Expression b) {
    auto& s=*state_;if(!s.open||!a||a>=s.nodes.size()||b>=s.nodes.size())return 0;
    return s.add({State::Kind::Arithmetic,op,a,b,0,0});
}
void MinigameEquations::bind_snapshot(Expression expression,std::uint32_t snapshot,std::uint32_t field) {
    auto& s=*state_;if(!s.open)return;
    std::vector<Expression> todo{expression};std::vector<bool> seen(s.nodes.size());
    while(!todo.empty()) {
        const auto id=todo.back();todo.pop_back();
        if(!id||id>=s.nodes.size()||seen[id])continue;
        seen[id]=true;auto& n=s.nodes[id];
        if(n.kind==State::Kind::Load&&n.address==snapshot)n.address=field;
        if(n.kind==State::Kind::Arithmetic||n.kind==State::Kind::Anchored) {todo.push_back(n.a);todo.push_back(n.b);}
    }
}
void MinigameEquations::store_float(std::uint32_t address,Expression expression,float number,
                                   FieldRole role,const Lifetime& owner) {
    auto& s=*state_;if(!s.open||!expression||expression>=s.nodes.size()||!field_pointer(address))return;
    if(role!=FieldRole::Scratch&&owner.scene!=s.scene)return;
    if(role==FieldRole::Command) {
        expression=s.add({State::Kind::Constant,Operation::Add,0,0,0,number});role=FieldRole::Scratch;
    }
    const auto raw=s.recurrence(address,expression,value(s.ram,address),role);
    const auto lifted=role==FieldRole::Scratch?raw:
        s.add({State::Kind::Anchored,role==FieldRole::Angle?Operation::AngleWrap:Operation::Add,raw,0,address,number});
    s.memory[address]=lifted;s.memory_values[address]=std::bit_cast<std::uint32_t>(number);
    if(role==FieldRole::Scratch) {
        for(auto& field:s.fields)if(field.address==address)field.role=FieldRole::Scratch;
        return;
    }
    auto at=std::find_if(s.fields.begin(),s.fields.end(),[&](const auto& f){return f.address==address;});
    State::Field field{address,std::bit_cast<std::uint32_t>(number),std::bit_cast<std::uint32_t>(number),lifted,role,owner};
    if(at==s.fields.end())s.fields.push_back(field);else *at=field;
}
void MinigameEquations::constrain_float(std::uint32_t address,double minimum,double maximum,bool clamp) {
    auto& s=*state_;if(!s.open||minimum>maximum)return;
    for(auto& field:s.fields)if(field.address==address) {
        field.minimum=minimum;field.maximum=maximum;field.clamp=clamp;
    }
}
std::vector<std::uint32_t> MinigameEquations::changed_fields() const {return state_->changed_addresses;}
void MinigameEquations::command_write(std::uint32_t address,unsigned bytes) {
    auto& s=*state_;if(!s.open)return;
    for(auto& field:s.fields)
        if(std::uint64_t(field.address)<std::uint64_t(address)+bytes&&std::uint64_t(address)<std::uint64_t(field.address)+4)
            field.role=FieldRole::Scratch;
    for(unsigned i=0;i<bytes;++i) {
        const auto aligned=(address+i)&~3u;s.memory.erase(aligned);s.memory_values.erase(aligned);
    }
}
std::vector<std::uint32_t> MinigameEquations::changed_local_actors() const {return state_->changed_actors;}
void MinigameEquations::end_source(std::uint8_t* ram,Nanoseconds now) {
    auto& s=*state_;if(!s.open||s.ram!=ram)return;
    s.open=false;s.source_time=s.shown_time=now;
    for(const auto& node:s.nodes)
        if(node.kind==State::Kind::Load&&field_pointer(node.address))
            s.source_values[node.address]=value(ram,node.address);
    for(auto& f:s.fields) {
        if(!s.valid(f,ram)||bits(ram,f.address)!=f.source)f.role=FieldRole::Scratch;
        if(f.role!=FieldRole::Scratch)s.source_values[f.address]=value(ram,f.address);
    }
    for(Expression id=1;id<s.nodes.size();++id) {
        auto& node=s.nodes[id];
        if(node.kind!=State::Kind::Load)continue;
        const auto field=std::find_if(s.fields.begin(),s.fields.end(),[&](const auto& f){return f.address==node.address&&f.role!=FieldRole::Scratch;});
        if(field==s.fields.end()||s.reads(field->expression,node.address)||s.reaches(field->expression,id))continue;
        node.kind=State::Kind::Reference;node.a=field->expression;
    }
}
void MinigameEquations::sample(std::uint8_t* ram,Nanoseconds now) {
    auto& s=*state_;if(s.open||s.ram!=ram||now<s.source_time||now<=s.shown_time)return;
    auto* rdram=ram;
    const double h=std::clamp(double(now-s.source_time)*60.0/(double(second)*s.divider),0.0,1.0);
    s.restore(ram);s.changed_actors.clear();s.changed_addresses.clear();
    std::vector<double> at(s.nodes.size(),std::numeric_limits<double>::quiet_NaN());
    std::vector<double> base(s.nodes.size(),std::numeric_limits<double>::quiet_NaN());
    for(auto& f:s.fields) {
        if(f.role==FieldRole::Scratch||!s.valid(f,ram))continue;
        const double initial=std::bit_cast<float>(f.source);
        double delta=s.evaluate(f.expression,h,at)-s.evaluate(f.expression,0,base);
        if(f.role==FieldRole::Angle)delta=std::remainder(delta,360.0);
        if((initial<=f.minimum&&delta<0)||(initial>=f.maximum&&delta>0))delta=0;
        const double constrained=f.clamp?std::clamp(initial+delta,f.minimum,f.maximum):initial+delta;
        const float next=float(f.role==FieldRole::Angle?wrapped(constrained):constrained);
        if(!std::isfinite(next))continue;
        f.published=std::bit_cast<std::uint32_t>(next);
        MEM_W(0,std::int32_t(f.address))=std::int32_t(f.published);
        if(f.published!=f.source)s.changed_addresses.push_back(f.address);
        if(f.published!=f.source&&f.owner.actor&&f.address-f.owner.actor<=0x20&&
           std::find(s.changed_actors.begin(),s.changed_actors.end(),f.owner.actor)==s.changed_actors.end())
            s.changed_actors.push_back(f.owner.actor);
    }
    s.shown_time=now;
}
void MinigameEquations::resume_after_handoff(std::uint8_t* ram,Nanoseconds now) {
    auto& s=*state_;
    if(s.ram!=ram)return;
    if(s.open||now<s.source_time)throw std::logic_error("Minigame handoff outside completed Source boundary");
    s.restore(ram);s.source_time=s.shown_time=now;
    s.changed_actors.clear();s.changed_addresses.clear();
}
void MinigameEquations::restore_source(std::uint8_t* ram){state_->restore(ram);}
void MinigameEquations::reset(std::uint8_t* ram) {
    if(ram)state_->restore(ram);
    state_=std::make_unique<State>();
}
bool MinigameEquations::recording() const {return state_->open;}
std::uint32_t MinigameEquations::module() const {return state_->rom;}
}
