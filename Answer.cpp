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
#include <iostream>

using namespace std;

namespace {
    using namespace hpc;
}

namespace hpc {
    int prevLotus;
    float baseAccessTiming;
    int lotusLen;
    bool isChange;
    Vec2 flow;
    
    
    /* アクセルタイミング */
    Vec2 prevAccelPos;
    float accelLength;
    float accelTTL;
    float accelTimingDist;
    int accelFrame;
    int accelNum;
    int framesFromLastAccel;
    
    
    /* 目標ポジション */
    Vec2 lotusTargetPos[1000];
    
    /* helper */
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
    
    Vec2 lastPos(Vec2 pos, Vec2 vel) {
        while (vel.length() > 0) {
            vel = decel(vel);
            pos += vel + flow;
        }
        return pos;
    }
    
    /* */
    // 何フレームでいけるか
    bool canReachFrame (int frame, Chara player, Circle target) {
        Vec2 targetPos = target.pos() - (flow * frame);
        Vec2 vel = nomarizeVel(player.pos(), targetPos);
        Vec2 pos = player.pos();
        for(int i = 0; i < frame; i++) {
            pos += vel;
            vel = decel(vel);
        }
        
        return Collision::IsHit(target, player.region(), pos);
    }
    
    // Nアクセルでいけるか いけないなら-1
    int canReachAccel (int accel, Chara player, Vec2 target) {
        
        for (int f = 1; f < accelTTL; f++) {
            Vec2 nextPos = (target - player.pos()) / accel;
            Circle targetCircle(player.pos() + nextPos, 0.5);
            if (canReachFrame(f, player, targetCircle)) {
                return f;
            }
        }
        return -1;
    }

    
    
    /* アクセルタイミング */
    // 1アクセルで何ターン生き延びるか
    void setAccelTTL() {
        float s = Parameter::CharaAccelSpeed();
        accelTTL = 0;
        while (s > 0) {
            s -= Parameter::CharaDecelSpeed();
            accelTTL += 1;
        }
    }
    
    // 1アクセルでどれくらい進めるか
    void setAccelLength() {
        float tmp = Parameter::CharaAccelSpeed();
        accelLength = 0;
        while (tmp > 0) {
            accelLength += tmp;
            tmp -= Parameter::CharaDecelSpeed();
        }
    }
    
    
    int numTurn(float dist) {
        int i = 0;
        float speed = Parameter::CharaAccelSpeed();
        while (dist > 0) {
            i++;
            dist -= speed;
            speed -= Parameter::CharaDecelSpeed();
        }
        return i;
    }
    
   
    Vec2 accelLastPos(Vec2 pos, Vec2 targetPos) {
        Vec2 vel = nomarizeVel(pos, targetPos);
        pos += vel + flow;
        while (vel.length() >= baseAccessTiming) {
            vel = decel(vel);
            pos += vel + flow;
        }
        return pos;
    }
    
    
    void setAccelFrame(Chara player, int lotusNum){
        int frameNum = 0;
        for (int i = 1; i < 100; i++) {
            frameNum = canReachAccel(i, player, lotusTargetPos[lotusNum]);
            if (frameNum != -1) {
                accelNum = i;
                accelFrame = frameNum;
                return;
            }
        }
    }
    
    Vec2 getNextPosition(Vec2 pos, Vec2 vel, bool useAccel, Vec2 targetPos) {
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
    bool gettingCloser(Chara player, Circle lotus) {
        if (player.vel().equals(Vec2(0, 0))) {
            return Collision::IsHit(lotus, player.region(), player.pos() + (flow * 5));
        }
        
        Vec2 nextPos = getNextPosition(player.pos(), player.vel(), false, Vec2(0, 0));

        Vec2 currentVec = lotus.pos() - player.pos();
        Vec2 nextVec = lotus.pos() - nextPos;
        return (currentVec.length() - nextVec.length()) / player.vel().length() > 0.7;
    }
    
    Vec2 getTargetPos(Chara player, Circle c1, int i) {
        Vec2 v1 = lotusTargetPos[i % lotusLen];
        Vec2 v2 = lotusTargetPos[(i + 1) % lotusLen];

        if (Collision::IsHit(c1, player.region(), v2)) { // 2つめのに行くと1つ目に当たるなら2つ目のに向かう
            Vec2 vel = (v2 - player.pos()).getNormalized(Parameter::CharaAccelSpeed()) - flow;
            return player.pos() + vel * Parameter::CharaAddAccelWaitTurn;
        }
        
//        Vec2 target;
//        if (accelNum > 1) {
//            Vec2 d = (v1 - player.pos()) / Math::Max(accelNum, 1);
//            target = player.pos() + d - (flow * accelFrame);
//        } else {
//            target = v1 - (flow * accelFrame);
//        }
//        
//        
//        return target;
        Vec2 vel = (v1 - player.pos()).getNormalized(Parameter::CharaAccelSpeed()) - flow;
        return player.pos() + vel * Parameter::CharaAddAccelWaitTurn;
    }
    
    Vec2 getCenterLotusPos(const Chara player, const Vec2 p1, const Circle c2, const Vec2 p3) {
        Vec2 v1 = p1 - c2.pos();
        Vec2 v2 = p3 - c2.pos();
        float v1v2Angle = (Math::ATan2(v2.y, v2.x) - Math::ATan2(v1.y, v1.x));
        if (v1v2Angle > Math::PI) v1v2Angle -= (Math::PI * 2);
        if (v1v2Angle < -Math::PI) v1v2Angle += (Math::PI * 2);
        v1v2Angle /= 2;
        
        
        if (!Math::IsValid(v1v2Angle)) {
            v1v2Angle = 0;
        }
        
        float horizontalV2Angle = Math::ATan2(0, 1) - Math::ATan2(v2.y, v2.x);
        float angle = horizontalV2Angle + v1v2Angle;
        
        return c2.pos() + (c2.radius() + player.region().radius() * 0.5) * Vec2(Math::Cos(angle), -Math::Sin(angle));
        
    }
    
    
    void setLotusTargetPos(const StageAccessor& aStageAccessor){
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        Chara player = aStageAccessor.player();
        
        lotusTargetPos[0] = getCenterLotusPos(player,
                                              lotuses[(- 1 + lotusLen) % lotusLen].pos(),
                                              lotuses[0].region(),
                                              lotuses[(1 + lotusLen) % lotusLen].pos());
        
        // 仮位置の決定
        for (int i = 1; i < lotusLen; i++) {
            lotusTargetPos[i] = getCenterLotusPos(player,
                                                  lotusTargetPos[(i - 1 + lotusLen) % lotusLen],
                                                  lotuses[(i) % lotusLen].region(),
                                                  lotuses[(i + 1) % lotusLen].pos());
        }
        
        // 何度かループを回して最適な位置を決める
        for (int k = 0; k < 10; k++) {
            for (int i = 0; i < lotusLen; i++) {
                lotusTargetPos[i] = getCenterLotusPos(player,
                                                      lotusTargetPos[(i - 1 + lotusLen) % lotusLen],
                                                      lotuses[(i) % lotusLen].region(),
                                                      lotusTargetPos[(i + 1) % lotusLen]);
            }
        }

    }
    
    
    /////////////////////////////////////////////////////////////////////////////////////
    

    /// 各ステージ開始時に呼び出されます。
    /// この関数を実装することで、各ステージに対して初期処理を行うことができます。
    /// @param[in] aStageAccessor 現在のステージ。
    void Answer::Init(const StageAccessor& aStageAccessor) {
        prevLotus = aStageAccessor.player().targetLotusNo();
        lotusLen = aStageAccessor.lotuses().count();
        flow = aStageAccessor.field().flowVel();
        Chara player = aStageAccessor.player();
        isChange = true;

        prevLotus = -1;
        prevAccelPos = player.pos();

        // 蓮ポジションの決定
        setLotusTargetPos(aStageAccessor);
        setAccelLength();
        setAccelTTL();

        baseAccessTiming = Parameter::CharaAccelSpeed();
        for (int i=0; i < Parameter::CharaAddAccelWaitTurn; i++) { baseAccessTiming -=  Parameter::CharaDecelSpeed();}
        
        setAccelFrame(player, 0);
        
    }

    /// 各ターンでの動作を返します。
    /// @param[in] aStageAccessor 現在ステージの情報。
    /// @return これから行う動作を表す Action クラス。
    Action Answer::GetNextAction(const StageAccessor& aStageAccessor) {
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        const Chara& player = aStageAccessor.player();
        
        Lotus targetLotus = lotuses[player.targetLotusNo()];
        
        if (prevLotus != player.targetLotusNo()) {
            isChange = true;
        }
        
        bool isAccel = false;
        if (isChange){
            isAccel = !gettingCloser(player, targetLotus.region());
            if (isAccel) {
                setAccelFrame(player, player.targetLotusNo());
                isChange = false;
            }
        } else {
            isAccel = !Collision::IsHit(targetLotus.region(), player.region(), lastPos(player.pos(), player.vel()))
            and ( (framesFromLastAccel >= accelFrame
                 or player.vel().length() < Parameter::CharaDecelSpeed()) );

            if (isAccel) {
                if (player.accelCount() <= 1) {
                    isAccel = false;
                }
            }
        }

        Vec2 targetPos = getTargetPos(player, targetLotus.region(), player.targetLotusNo());
        
        framesFromLastAccel++;
        prevLotus = player.targetLotusNo();
        if (player.accelCount() > 0 && isAccel) {
            framesFromLastAccel = 0;
            accelNum--;
            return Action::Accel(targetPos);
        }
        return Action::Wait();
    }
    
    
}





//------------------------------------------------------------------------------
// EOF
