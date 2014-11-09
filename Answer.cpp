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
    int lotusLen;
    
    const Vec2 nullVec2 = Vec2(-1, -1);
    Vec2 lotusTargetPos[1000];
    
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
    
   
    ///今のままのベクトルでも、次の蓮に近づくかどうか
    bool gettingCloser(Chara player, Circle lotus, Vec2 flow) {
        Vec2 nextPos = getNextPosition(player.pos(), player.vel(), flow, false, Vec2(0, 0));

        Vec2 currentVec = lotus.pos() - player.pos();
        Vec2 nextVec = lotus.pos() - nextPos;
        return (currentVec.length() - nextVec.length()) / player.vel().length() > 0.7;
    }
    
    Vec2 getTargetPos(Chara player, Circle c1, int i, Vec2 flow) {
        Vec2 v1 = lotusTargetPos[i % lotusLen];
        Vec2 v2 = lotusTargetPos[(i + 1) % lotusLen];

        if (Collision::IsHit(c1, player.region(), v2)) { // 2つめのに行くと1つ目に当たるなら2つ目のに向かう
            Vec2 vel = (v2 - player.pos()).getNormalized(Parameter::CharaAccelSpeed()) - flow;
            return player.pos() + vel * Parameter::CharaAddAccelWaitTurn;
        }
        
        Vec2 vel = (v1 - player.pos()).getNormalized(Parameter::CharaAccelSpeed()) - flow;
        return player.pos() + vel * Parameter::CharaAddAccelWaitTurn;
    }
    
    Vec2 getCenterLotusPos(const Chara player, const Circle c1, const Circle c2, const Circle c3) {
        Vec2 v1 = c1.pos() - c2.pos();
        Vec2 v2 = c3.pos() - c2.pos();
        float v1v2Angle = (Math::ATan2(v2.y, v2.x) - Math::ATan2(v1.y, v1.x)) / 2;
        
        if (!Math::IsValid(v1v2Angle)) {
            v1v2Angle = 0;
        }
        
        float horizontalV2Angle = Math::ATan2(0, 1) - Math::ATan2(v2.y, v2.x);
        float angle = horizontalV2Angle + v1v2Angle;
        
        
        return c2.pos() + (c2.radius() + player.region().radius() * 0.8) * Vec2(Math::Cos(angle), -Math::Sin(angle));
    }
    
    
    void setLotusTargetPos(const StageAccessor& aStageAccessor){
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        Chara player = aStageAccessor.player();
        
        for (int i = 0; i < lotusLen; i++) {
            lotusTargetPos[i] = getCenterLotusPos(player,
                                                  lotuses[(i - 1 + lotusLen) % lotusLen].region(),
                                                  lotuses[(i) % lotusLen].region(),
                                                  lotuses[(i + 1) % lotusLen].region());
        }
    }
    

    /// 各ステージ開始時に呼び出されます。
    /// この関数を実装することで、各ステージに対して初期処理を行うことができます。
    /// @param[in] aStageAccessor 現在のステージ。
    void Answer::Init(const StageAccessor& aStageAccessor) {
        prevLotus = aStageAccessor.player().targetLotusNo();
        sTimer = 0;
        lotusLen = aStageAccessor.lotuses().count();
        
        baseAccessTiming = Parameter::CharaAccelSpeed();
        for (int i=0; i < Parameter::CharaAddAccelWaitTurn; i++) { baseAccessTiming -=  Parameter::CharaDecelSpeed();}
        
        setLotusTargetPos(aStageAccessor);
        
        
    
        
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
//        Lotus nextTargetLotus = lotuses[(player.targetLotusNo() + 1) % lotuses.count()];
        
        
        bool isAccel = false;
        if (Collision::IsHit(lotuses[prevLotus].region(), player.region())){
            isAccel = !gettingCloser(player, targetLotus.region(), flow);
        } else {
            isAccel = !Collision::IsHit(targetLotus.region(), player.region(), lastPos(player.pos(), player.vel(), flow))
            and player.vel().length() < baseAccessTiming;
            if (isAccel) {
                if (player.accelCount() <= 1) {
                    isAccel = false;
                }
            }
        }

        Vec2 targetPos = getTargetPos(player, targetLotus.region(), player.targetLotusNo(), flow);
        
        prevLotus = player.targetLotusNo();
        if (player.accelCount() > 0 && isAccel) {
            return Action::Accel(targetPos);
        }
        return Action::Wait();
    }
    
    
}





//------------------------------------------------------------------------------
// EOF
