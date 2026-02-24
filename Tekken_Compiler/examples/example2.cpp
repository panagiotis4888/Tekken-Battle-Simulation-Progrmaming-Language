
#include "../include/Tekken.h"

BEGIN_GAME

CREATE ABILITY {
    NAME: "Quick_Jab",
    ACTION: START
        DAMAGE DEFENDER 10
    END
}

CREATE ABILITY {
    NAME: "Power_Slam",
    ACTION: START
        DAMAGE DEFENDER 25
    END
}

CREATE ABILITY {
    NAME: "Meditate",
    ACTION: START
        HEAL ATTACKER 15
    END
}

CREATE ABILITY {
    NAME: "Combo_Strike",
    ACTION: START
        IF GET_HP(DEFENDER) > 50 DO
            DAMAGE DEFENDER 20
        ELSE
            DAMAGE DEFENDER 35
        END
    END
}

CREATE FIGHTER {
    NAME: "Ryu",
    TYPE: "Rushdown",
    HP: 100
}

CREATE FIGHTER {
    NAME: "Zangief",
    TYPE: "Grappler",
    HP: 120
}

DEAR "Ryu" LEARN [
    ABILITY_NAME(Quick_Jab)
    ABILITY_NAME(Combo_Strike)
    ABILITY_NAME(Meditate)
]

DEAR "Zangief" LEARN [
    ABILITY_NAME(Power_Slam)
    ABILITY_NAME(Meditate)
]

DUEL

END_GAME
