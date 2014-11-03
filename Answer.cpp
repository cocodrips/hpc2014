//------------------------------------------------------------------------------
/// @file
/// @brief    HPCAnswer.hpp の実装 (解答記述用ファイル)
/// @author   ハル研究所プログラミングコンテスト実行委員会
///
/// @copyright  Copyright (c) 2014 HAL Laboratory, Inc.
/// @attention  このファイルの利用は、同梱のREADMEにある
///             利用条件に従ってください

//------------------------------------------------------------------------------

// Answer.cpp 専用のインクルードファイルです。
// 別のファイルをインクルードした場合、評価時には削除されます。
#include "HPCAnswerInclude.hpp"

namespace {
    int sTimer = 0;
    using namespace hpc;
}

namespace hpc {
    int prevLotus;
    float baseAccessTiming;
    Vec2 decel(Vec2 vel) {
        if (vel.length() <= Parameter::CharaDecelSpeed()) {
            vel = Vec2(0, 0);
        } else {
            vel *= (vel.length() - Parameter::CharaDecelSpeed()) / vel.length();
        }
        return vel;
    }
    
    Vec2 nomarizeVel(Vec2 pos, Vec2 targetPos) {
        Vec2 toPos = targetPos - pos;
        toPos /= toPos.length();
        toPos *= Parameter::CharaAccelSpeed();
        return toPos;
    }
    
    Vec2 lastPos(Vec2 pos, Vec2 vel, Vec2 flow) {
        while (vel.length() > 0) {
            vel = decel(vel);
            pos += vel + flow;
        }
        return pos;
    }
    
    Vec2 getNextPosition(Vec2 pos, Vec2 vel, Vec2 flow, bool useAccel, Vec2 targetPos) {
        if (useAccel) {
            vel = nomarizeVel(pos, targetPos);
        } else {
            vel = decel(vel);
        }
        return pos + vel + flow;
    }
    
    float distTo2Vec(Vec2 c1, Vec2 c2, Vec2 c3) {
        return ((c3 - c1) + (c2 - c1)).length();
    }
    
    Vec2 getTargetPos(Chara player, Circle c1, Circle c2, Vec2 flow) {
        // この関数は波を考慮していない

        if (Collision::IsHit(c1, player.region(), c2.pos())) {
            Vec2 vel = (c2.pos() - player.pos()).getNormalized(Parameter::CharaAccelSpeed()) - flow;
            return player.pos() + vel * Parameter::CharaAddAccelWaitTurn;
        }
        
        Vec2 playerToC2Vec = c2.pos() - player.pos();
        Vec2 normalVec = Vec2(playerToC2Vec.y, -playerToC2Vec.x);
        

        Vec2 n1 = normalVec;
        Vec2 n2 = -normalVec;

        n1.normalize(player.region().radius() + c1.radius() * 0.5);
        n2.normalize(player.region().radius() + c1.radius() * 0.5);
        
        if (distTo2Vec(c1.pos() + n1, player.pos(), c2.pos())
            < distTo2Vec(c1.pos() + n2, player.pos(), c2.pos())) {
            return c1.pos() + n1;
        }
        
        return c1.pos() + n2;
    }
    


    /// 各ステージ開始時に呼び出されます。
    /// この関数を実装することで、各ステージに対して初期処理を行うことができます。
    /// @param[in] aStageAccessor 現在のステージ。
    void Answer::Init(const StageAccessor& aStageAccessor) {
        prevLotus = aStageAccessor.player().targetLotusNo();
        sTimer = 0;
        baseAccessTiming = Parameter::CharaAccelSpeed();
        for (int i=0; i < Parameter::CharaAddAccelWaitTurn; i++) { baseAccessTiming -=  Parameter::CharaDecelSpeed();}
        
    }

    /// 各ターンでの動作を返します。
    /// @param[in] aStageAccessor 現在ステージの情報。
    /// @return これから行う動作を表す Action クラス。
    Action Answer::GetNextAction(const StageAccessor& aStageAccessor) {
        ++sTimer; // turn_counter
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        const Chara& player = aStageAccessor.player();
        
        Vec2 flow = aStageAccessor.field().flowVel();
        Lotus targetLotus = lotuses[player.targetLotusNo()];
        Lotus nextTargetLotus = lotuses[(player.targetLotusNo() + 1) % lotuses.count()];
        
        
        bool isAccel = false;
        if (Collision::IsHit(lotuses[prevLotus].region(), player.region())){
            isAccel = true;
        } else {
            isAccel = !Collision::IsHit(targetLotus.region(), player.region(), lastPos(player.pos(), player.vel(), flow))
            and player.vel().length() < baseAccessTiming;
            if (isAccel) {
                if (player.accelCount() <= 1) {
                    isAccel = false;
                }
            }
        }
        
        

//        Vec2 targetPos = targetLotus.pos() - flow;
        Vec2 targetPos = getTargetPos(player, targetLotus.region(), nextTargetLotus.region(), flow);
//        targetPos -= Parameter::CharaAddAccelWaitTurn * flow;
        
        prevLotus = player.targetLotusNo();
        if (player.accelCount() > 0 && isAccel) {
            return Action::Accel(targetPos);
        }
//        getNextPosition(aStageAccessor);
        return Action::Wait();
        
    }
    
    
    
    
    
}





//------------------------------------------------------------------------------
// EOF
