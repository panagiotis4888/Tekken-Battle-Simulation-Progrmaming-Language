#ifndef TEKKEN_H
#define TEKKEN_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <initializer_list>

enum class FighterType { Rushdown, Grappler, Heavy, Evasive };

inline FighterType strToType(const std::string& s) {
    if (s == "Rushdown") return FighterType::Rushdown;
    if (s == "Grappler") return FighterType::Grappler;
    if (s == "Heavy")    return FighterType::Heavy;
    if (s == "Evasive")  return FighterType::Evasive;
    throw std::invalid_argument("Invalid type: " + s);
}

inline std::string typeToStr(FighterType t) {
    switch(t) {
        case FighterType::Rushdown: return "Rushdown";
        case FighterType::Grappler: return "Grappler";
        case FighterType::Heavy: return "Heavy";
        case FighterType::Evasive: return "Evasive";
    }
    return "";
}

class Fighter {
    std::string name_; FighterType type_; int maxHP_, hp_; bool inRing_;
    std::vector<std::string> abilities_;
public:
    Fighter() : type_(FighterType::Rushdown), maxHP_(100), hp_(100), inRing_(true) {}
    Fighter(const std::string& n, const std::string& t, int h)
        : name_(n), type_(strToType(t)), maxHP_(h), hp_(h), inRing_(true) {}
    const std::string& getName() const { return name_; }
    std::string getTypeString() const { return typeToStr(type_); }
    FighterType getType() const { return type_; }
    int getHP() const { return hp_; }
    int getMaxHP() const { return maxHP_; }
    bool isOutOfRing() const { return !inRing_; }
    void takeDamage(int a) { hp_ -= a; if(hp_ < 0) hp_ = 0; }
    void heal(int a) { hp_ += a; if(hp_ > maxHP_) hp_ = maxHP_; }
    void setInRing(bool v) { inRing_ = v; }
    void addAbility(const std::string& a) { abilities_.push_back(a); }
    const std::vector<std::string>& getAbilities() const { return abilities_; }
    
    double getOutgoingMod(const Fighter& target, int round) const {
        double mod = 1.0;
        switch(type_) {
            case FighterType::Rushdown:
                mod = (target.type_ == FighterType::Grappler) ? 1.20 : 1.15;
                break;
            case FighterType::Evasive:
                mod = 1.07;
                break;
            case FighterType::Grappler:
                if(round % 2 == 1) mod = 1.07;
                break;
            default: break;
        }
        return mod;
    }
    
    double getIncomingMod(const Fighter& attacker) const {
        double mod = 1.0;
        switch(type_) {
            case FighterType::Heavy:
                mod = (attacker.type_ == FighterType::Evasive) ? 0.70 : 0.80;
                break;
            case FighterType::Evasive:
                mod = 0.93;
                break;
            default: break;
        }
        return mod;
    }
    
    void applyGrapplerBonus(int round) {
        if(type_ == FighterType::Grappler && round % 2 == 0 && round > 0) {
            heal(static_cast<int>(maxHP_ * 0.05));
        }
    }
};

inline std::map<std::string, Fighter>& g_fighters() { 
    static std::map<std::string, Fighter> m; return m; 
}
inline void regFighter(const Fighter& f) { g_fighters()[f.getName()] = f; }
inline Fighter& getFighter(const std::string& n) { return g_fighters().at(n); }

class ActionContext;
using AbilityAction = std::function<void(Fighter&, Fighter&, int, ActionContext&)>;
struct Ability { std::string name; AbilityAction action; };

inline std::map<std::string, Ability>& g_abilities() { 
    static std::map<std::string, Ability> m; return m; 
}
inline void regAbility(const std::string& n, AbilityAction a) { 
    g_abilities()[n] = {n, std::move(a)}; 
}

class ActionContext {
    std::vector<std::pair<int, std::function<void()>>> forActs_, afterActs_;
    int round_ = 0;
public:
    void scheduleFor(int r, std::function<void()> a) { 
        if(r>0) forActs_.push_back({r,std::move(a)}); 
    }
    void scheduleAfter(int r, std::function<void()> a) { 
        if(r>0) afterActs_.push_back({round_+r,std::move(a)}); 
    }
    void processRound(int r) {
        round_ = r;
        for(auto it=forActs_.begin(); it!=forActs_.end();) {
            if(it->first>0){it->second();it->first--;}
            it = (it->first<=0) ? forActs_.erase(it) : it+1;
        }
        for(auto it=afterActs_.begin(); it!=afterActs_.end();)
            if(it->first==r){it->second();it=afterActs_.erase(it);} else ++it;
    }
    void clear() { forActs_.clear(); afterActs_.clear(); round_=0; }
};

inline void resetGame() { g_fighters().clear(); g_abilities().clear(); }

inline void printFighterStatus(const Fighter& f, bool wasOutOfRing, bool isNowOutOfRing) {
    std::cout << "\n##########################\n";
    std::cout << "Name: " << f.getName() << "\n";
    std::cout << "HP: " << f.getHP() << "\n";
    std::cout << "Type: " << f.getTypeString() << "\n";
    if (isNowOutOfRing && !wasOutOfRing) {
        std::cout << "fighter exits the ring\n";
    } else if (!isNowOutOfRing && wasOutOfRing) {
        std::cout << "fighter enters the ring\n";
    } else if (!isNowOutOfRing) {
        std::cout << "fighter enters the ring\n";
    } else {
        std::cout << "fighter exits the ring\n";
    }
    std::cout << "##########################\n";
}

inline void runDuel() {
    auto& fighters = g_fighters();
    auto& abilities = g_abilities();
    
    std::vector<std::string> names;
    for(auto& p : fighters) {
        if(p.first != "_DUMMY_" && p.first != "_END_") names.push_back(p.first);
    }
    
    if(names.empty()) {
        std::cout << "No fighters available!\n";
        return;
    }
    
    std::cout << "-----------------------------FIGHTER THE GAME-------------------------------\n\n";
    
    std::cout << "Player1 select fighter:\n";
    std::cout << "------------------------\n";
    for(size_t i = 0; i < names.size(); ++i) {
        std::cout << names[i] << "\n";
    }
    std::cout << "------------------------\n";
    
    std::string p1Name;
    std::getline(std::cin >> std::ws, p1Name);
    
    std::cout << "\nPlayer2 select fighter:\n";
    std::cout << "------------------------\n";
    for(size_t i = 0; i < names.size(); ++i) {
        std::cout << names[i] << "\n";
    }
    std::cout << "------------------------\n";
    
    std::string p2Name;
    std::getline(std::cin >> std::ws, p2Name);
    
    if(fighters.find(p1Name) == fighters.end() || fighters.find(p2Name) == fighters.end()) {
        std::cout << "Invalid fighter selection!\n";
        return;
    }
    
    Fighter f1 = fighters[p1Name];
    Fighter f2 = fighters[p2Name];
    
    ActionContext ctx1, ctx2;
    int round = 1;
    
    while(true) {
        std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        std::cout << "Round " << round << "\n";
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
        
        f1.applyGrapplerBonus(round);
        f2.applyGrapplerBonus(round);
        ctx1.processRound(round);
        ctx2.processRound(round);
        
        if(f1.getHP() <= 0) { std::cout << "\n" << f2.getName() << " WINS!\n"; break; }
        if(f2.getHP() <= 0) { std::cout << "\n" << f1.getName() << " WINS!\n"; break; }
        
        if(f1.isOutOfRing()) {
            std::cout << "\n" << f1.getName() << "(Player1) has not a fighter that can enter the ring so he can't cast an ability.\n";
        } else {
            std::cout << "\n" << f1.getName() << "(Player1) select ability:\n";
            std::cout << "------------------------\n";
            auto& abs1 = f1.getAbilities();
            for(size_t i = 0; i < abs1.size(); ++i) {
                std::cout << abs1[i] << "\n";
            }
            std::cout << "------------------------\n";
            
            std::string abilityName;
            std::getline(std::cin >> std::ws, abilityName);
            
            bool hasAbility = false;
            for(const auto& a : abs1) { if(a == abilityName) hasAbility = true; }
            if(hasAbility && abilities.count(abilityName)) {
                abilities[abilityName].action(f1, f2, round, ctx1);
            }
            
            printFighterStatus(f2, false, f2.isOutOfRing());
            printFighterStatus(f1, false, f1.isOutOfRing());
        }
        
        if(f2.getHP() <= 0) { std::cout << "\n" << f1.getName() << " WINS!\n"; break; }
        
        if(f2.isOutOfRing()) {
            std::cout << "\n" << f2.getName() << "(Player2) has not a fighter that can enter the ring so he can't cast an ability.\n";
        } else {
            std::cout << "\n" << f2.getName() << "(Player2) select ability:\n";
            std::cout << "------------------------\n";
            auto& abs2 = f2.getAbilities();
            for(size_t i = 0; i < abs2.size(); ++i) {
                std::cout << abs2[i] << "\n";
            }
            std::cout << "------------------------\n";
            
            std::string abilityName;
            std::getline(std::cin >> std::ws, abilityName);
            
            bool hasAbility = false;
            for(const auto& a : abs2) { if(a == abilityName) hasAbility = true; }
            if(hasAbility && abilities.count(abilityName)) {
                abilities[abilityName].action(f2, f1, round, ctx2);
            }
            
            printFighterStatus(f1, false, f1.isOutOfRing());
            printFighterStatus(f2, false, f2.isOutOfRing());
        }
        
        if(f1.getHP() <= 0) { std::cout << "\n" << f2.getName() << " WINS!\n"; break; }
        
        round++;
        if(round > 100) { std::cout << "Draw!\n"; break; }
    }
}

struct _FInit_ {
    const char* name;
    const char* type;
    int hp;
    _FInit_(const char* n, const char* t, int h) : name(n), type(t), hp(h) {}
    
    friend bool operator,(const _FInit_& f, bool b) {
        regFighter(Fighter(f.name, f.type, f.hp));
        return b;
    }
};

struct _AInit_ {
    const char* name;
    AbilityAction action;
    _AInit_(const char* n, AbilityAction a) : name(n), action(std::move(a)) {}
    
    friend bool operator,(const _AInit_& a, bool b) {
        regAbility(a.name, a.action);
        return b;
    }
};

struct _AbilityCollector_ {
    mutable std::vector<std::pair<const char*, AbilityAction>> entries;
    mutable bool finalized = false;
    
    _AbilityCollector_& operator,(_AInit_ a) {
        entries.push_back({a.name, std::move(a.action)});
        return *this;
    }
    
    void finalize() const {
        if(!finalized) {
            for(const auto& e : entries) {
                regAbility(e.first, e.second);
            }
            finalized = true;
        }
    }
    
    operator bool() const { 
        finalize();
        return false; 
    }
    
    friend bool operator,(const _AbilityCollector_& c, bool b) {
        c.finalize();
        return b;
    }
    
    ~_AbilityCollector_() {
        finalize();
    }
};

struct _AbilityListBuilder_ {
    _AbilityListBuilder_() {}
    
    _AbilityCollector_ operator[](_AInit_ first) {
        _AbilityCollector_ c;
        c.entries.push_back({first.name, std::move(first.action)});
        return c;
    }
};

struct _FighterCollector_ {
    mutable std::vector<_FInit_> entries;
    mutable bool finalized = false;
    
    _FighterCollector_& operator,(_FInit_ f) {
        entries.push_back(f);
        return *this;
    }
    
    void finalize() const {
        if(!finalized) {
            for(const auto& e : entries) {
                regFighter(Fighter(e.name, e.type, e.hp));
            }
            finalized = true;
        }
    }
    
    operator bool() const {
        finalize();
        return false;
    }
    
    friend bool operator,(const _FighterCollector_& c, bool b) {
        c.finalize();
        return b;
    }
    
    ~_FighterCollector_() {
        finalize();
    }
};

struct _FighterListBuilder_ {
    _FighterListBuilder_() {}
    
    _FighterCollector_ operator[](_FInit_ first) {
        _FighterCollector_ c;
        c.entries.push_back(first);
        return c;
    }
};

#define BEGIN_GAME int main() { resetGame(); if(false){}else if((_FInit_{"_DUMMY_","Rushdown",1}
#define END_GAME ,false)){} return 0; }
#define CREATE ,false)){}else if((
#define FIGHTER _FInit_
#define ABILITY _AInit_
#define ABILITIES _AbilityListBuilder_{}
#define FIGHTERS _FighterListBuilder_{}
#define NAME 0 ? (const char*)0
#define TYPE 0 ? (const char*)0
#define HP 0 ? 0
#define ACTION 0 ? (AbilityAction)nullptr
#define START [&](Fighter& _attacker_, Fighter& _defender_, int _round_, ActionContext& _ctx_) { \
    (void)_round_; (void)_ctx_; int _d_=0; {
#define END ;}}

struct _DmgFinal_ {
    Fighter& target;
    Fighter& attacker;
    int round;
    
    _DmgFinal_(Fighter& t, Fighter& a, int r) : target(t), attacker(a), round(r) {}
    
    void operator<<(int dmg) const {
        if(target.isOutOfRing()) {
            return;
        }
        int final_dmg = static_cast<int>(dmg * attacker.getOutgoingMod(target, round) * target.getIncomingMod(attacker));
        target.takeDamage(final_dmg);
    }
};

struct _DmgCmd_ {
    Fighter& attacker;
    int round;
    
    _DmgCmd_(Fighter& a, int r) : attacker(a), round(r) {}
    
    _DmgFinal_ operator<<(Fighter& target) const {
        return _DmgFinal_(target, attacker, round);
    }
};

struct _DmgOp_ { 
    Fighter* t; Fighter* a; int r;
    _DmgOp_(Fighter* target, Fighter* attacker, int round) : t(target), a(attacker), r(round) {}
    _DmgOp_& operator,(Fighter& f){t=&f;return *this;} 
    void operator,(int dmg){
        if(t) {
            int final_dmg = dmg;
            if(a) final_dmg = static_cast<int>(dmg * a->getOutgoingMod(*t, r) * t->getIncomingMod(*a));
            t->takeDamage(final_dmg);
        }
    }
};

struct _HealOp_ { 
    Fighter* t;
    _HealOp_() : t(nullptr) {}
    _HealOp_& operator,(Fighter& f){t=&f;return *this;} 
    void operator,(int amt){
        if(t) {
            t->heal(amt);
        }
    }
};

struct _HealFinal_ {
    Fighter& target;
    
    _HealFinal_(Fighter& t) : target(t) {}
    
    void operator<<(int amt) const {
        target.heal(amt);
    }
};

struct _HealCmd_ {
    _HealCmd_() {}
    
    _HealFinal_ operator<<(Fighter& target) const {
        return _HealFinal_(target);
    }
};

struct _TagOp_ { 
    Fighter* t;
    _TagOp_() : t(nullptr) {}
    _TagOp_& operator,(Fighter& f){t=&f;return *this;} 
    void operator,(bool v){if(t) t->setInRing(v);}
};

struct _TagAlphaValue_ {
    int dummy;
    _TagAlphaValue_() : dummy(0) {}
    _TagAlphaValue_ operator-() const { return *this; }
    _TagAlphaValue_& operator--() { return *this; }
};

static _TagAlphaValue_ _alpha_tag_;

struct _TagUnderscoreValue_ {
    int dummy;
    _TagUnderscoreValue_() : dummy(0) {}
};

static _TagUnderscoreValue_ _;

struct _TagFinal_ {
    Fighter& target;
    
    _TagFinal_(Fighter& t) : target(t) {}
    
    void operator<<(const _TagAlphaValue_&) const {
        target.setInRing(false);
    }
    
    void operator<<(const _TagUnderscoreValue_&) const {
        target.setInRing(true);
    }
};

struct _TagCmd_ {
    _TagCmd_() {}
    
    _TagFinal_ operator<<(Fighter& target) const {
        return _TagFinal_(target);
    }
};

#define DAMAGE ; _DmgCmd_{_attacker_, _round_} <<
#define DEFENDER _defender_ <<
#define ATTACKER _attacker_ <<
#define HEAL ; _HealCmd_{} <<
#define TAG ; _TagCmd_{} <<
#define SHOW ; std::cout <<

struct _GetEnd_ {};
static _GetEnd_ _get_end_;

struct _GetHPProxy_ {
    Fighter* f;
    _GetHPProxy_() : f(nullptr) {}
    _GetHPProxy_& operator<<(Fighter& fighter) { f = &fighter; return *this; }
    int operator<<(_GetEnd_) const { return f ? f->getHP() : 0; }
};

struct _GetTypeProxy_ {
    Fighter* f;
    _GetTypeProxy_() : f(nullptr) {}
    _GetTypeProxy_& operator<<(Fighter& fighter) { f = &fighter; return *this; }
    std::string operator<<(_GetEnd_) const { return f ? f->getTypeString() : ""; }
};

struct _GetNameProxy_ {
    Fighter* f;
    _GetNameProxy_() : f(nullptr) {}
    _GetNameProxy_& operator<<(Fighter& fighter) { f = &fighter; return *this; }
    const std::string& operator<<(_GetEnd_) const { 
        static std::string empty;
        return f ? f->getName() : empty; 
    }
};

struct _IsOutOfRingProxy_ {
    Fighter* f;
    _IsOutOfRingProxy_() : f(nullptr) {}
    _IsOutOfRingProxy_& operator<<(Fighter& fighter) { f = &fighter; return *this; }
    bool operator<<(_GetEnd_) const { return f ? f->isOutOfRing() : false; }
};

#define GET_HP(x) (_GetHPProxy_{} << x _get_end_)
#define GET_TYPE(x) (_GetTypeProxy_{} << x _get_end_)
#define GET_NAME(x) (_GetNameProxy_{} << x _get_end_)
#define IS_OUT_OF_RING(x) (_IsOutOfRingProxy_{} << x _get_end_)

#define AND(...) ([&]{ bool _args_[] = {__VA_ARGS__}; for(bool _b_ : _args_) if(!_b_) return false; return true; }())
#define OR(...) ([&]{ bool _args_[] = {__VA_ARGS__}; for(bool _b_ : _args_) if(_b_) return true; return false; }())
#define NOT(x) (!(x))

struct _ForScheduler_ {
    ActionContext* ctx;
    int rounds;
    Fighter* attacker;
    Fighter* defender;
    
    _ForScheduler_(ActionContext* c, int r, Fighter* a, Fighter* d) 
        : ctx(c), rounds(r), attacker(a), defender(d) {}
    
    template<typename F>
    void operator=(F&& action) {
        if(ctx && rounds > 0) {
            Fighter* a = attacker;
            Fighter* d = defender;
            auto act = action;
            ctx->scheduleFor(rounds, [=]() mutable {
                ActionContext dummyCtx;
                act(*a, *d, 0, dummyCtx);
            });
        }
    }
};

struct _AfterScheduler_ {
    ActionContext* ctx;
    int rounds;
    Fighter* attacker;
    Fighter* defender;
    
    _AfterScheduler_(ActionContext* c, int r, Fighter* a, Fighter* d) 
        : ctx(c), rounds(r), attacker(a), defender(d) {}
    
    template<typename F>
    void operator=(F&& action) {
        if(ctx && rounds > 0) {
            Fighter* a = attacker;
            Fighter* d = defender;
            auto act = action;
            ctx->scheduleAfter(rounds, [=]() mutable {
                ActionContext dummyCtx;
                act(*a, *d, 0, dummyCtx);
            });
        }
    }
};

#define FOR ; _ForScheduler_{&_ctx_,
#define AFTER ; _AfterScheduler_{&_ctx_,
#define ROUNDS , &_attacker_, &_defender_} = [&](Fighter& _attacker_, Fighter& _defender_, int _round_, ActionContext& _ctx_){ int _d2_=0; (void)_d2_; if(1
#define IF ;{if(
#define DO ){
#define ELSE ;}else{
#define ELSE_IF ;}else if(

struct _AbilityNameAdder_ {
    std::string name;
    _AbilityNameAdder_(const char* n) : name(n) {}
    _AbilityNameAdder_ operator+() const { return *this; }
};

struct _AbilityLearnCollector_ {
    std::vector<std::string> names;
    
    _AbilityLearnCollector_() {}
    _AbilityLearnCollector_(const std::string& n) { names.push_back(n); }
    
    _AbilityLearnCollector_& operator+(_AbilityNameAdder_ a) {
        names.push_back(a.name);
        return *this;
    }
};

inline _AbilityLearnCollector_ operator+(_AbilityNameAdder_ a, _AbilityNameAdder_ b) {
    _AbilityLearnCollector_ c;
    c.names.push_back(a.name);
    c.names.push_back(b.name);
    return c;
}

struct _LearnOp_ {
    const char* fighterName;
    
    _LearnOp_(const char* name) : fighterName(name) {}
    
    bool operator[](_AbilityNameAdder_ a) const {
        Fighter& f = getFighter(fighterName);
        f.addAbility(a.name);
        return false;
    }
    
    bool operator[](_AbilityLearnCollector_ c) const {
        Fighter& f = getFighter(fighterName);
        for (const auto& name : c.names) {
            f.addAbility(name);
        }
        return false;
    }
};

#define DEAR ,false)){}else if((_LearnOp_(
#define LEARN )
#define ABILITY_NAME(x) + _AbilityNameAdder_(#x)
#define DUEL ,false)){} runDuel(); if(false){}else if((_FInit_{"_END_","Rushdown",1}

#define α _alpha_tag_

#endif
