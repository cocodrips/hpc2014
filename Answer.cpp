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
    int firstLotus = 0;
    int prevLotus = 0;
    using namespace hpc;
}

namespace hpc {
    /// 各ステージ開始時に呼び出されます。
    /// この関数を実装することで、各ステージに対して初期処理を行うことができます。
    /// @param[in] aStageAccessor 現在のステージ。
    void Answer::Init(const StageAccessor& aStageAccessor)
    {
        prevLotus = firstLotus;
        sTimer = 0;
        
        
    }

    /// 各ターンでの動作を返します。
    /// @param[in] aStageAccessor 現在ステージの情報。
    /// @return これから行う動作を表す Action クラス。
    Action Answer::GetNextAction(const StageAccessor& aStageAccessor)
    {
        ++sTimer; // turn_counter
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        const Chara& player = aStageAccessor.player();
        
        Vec2 flow = aStageAccessor.field().flowVel();
        
        
        Lotus targetLotus = lotuses[player.targetLotusNo()];
        Vec2 nextVel = player.vel();
        nextVel.getNormalized(Parameter::CharaAccelSpeed());
        
        bool isAccel = !Collision::IsHit(targetLotus.region(), player.region(), player.pos() + nextVel + nextVel + nextVel + nextVel + nextVel)
        && player.vel().length() / Parameter::CharaAccelSpeed() < 0.5;
        
        Vec2 targetPos = targetLotus.pos() - flow;
        
        if (player.accelCount() > 0 && isAccel) {
            return Action::Accel(targetPos);
        }
        return Action::Wait();
    }
    
    
    
}





//------------------------------------------------------------------------------
// EOF
