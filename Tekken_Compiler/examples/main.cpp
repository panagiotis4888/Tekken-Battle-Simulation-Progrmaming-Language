#include "../include/Tekken.h"

BEGIN_GAME

CREATE ABILITY {
    NAME: "Give_Autographs",
    ACTION: START
        TAG DEFENDER ---α
        AFTER 2 ROUNDS DO
            TAG DEFENDER _
        END
    END
}

CREATE ABILITY {
    NAME: "Bleeding_Bite",
    ACTION: START
        FOR 5 ROUNDS DO
            DAMAGE DEFENDER 8
        END
    END
}

CREATE ABILITY {
    NAME: "Head_Smash",
    ACTION: START
        DAMAGE DEFENDER 22
    END
}

CREATE ABILITY {
    NAME: "Catch_A_Break",
    ACTION: START
        HEAL ATTACKER 30
    END
}

CREATE FIGHTER {
    NAME: "Lee",
    TYPE: "Rushdown",
    HP: 100
}

CREATE FIGHTER {
    NAME: "Jack-6",
    TYPE: "Heavy",
    HP: 90
}

DEAR "Lee" LEARN [
    ABILITY_NAME(Give_Autographs)
    ABILITY_NAME(Head_Smash)
    ABILITY_NAME(Catch_A_Break)
    ABILITY_NAME(Bleeding_Bite)
]

DEAR "Jack-6" LEARN [
    ABILITY_NAME(Head_Smash)
    ABILITY_NAME(Catch_A_Break)
    ABILITY_NAME(Bleeding_Bite)
]

DUEL

END_GAME
