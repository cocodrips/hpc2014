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

    /// 各ステージ開始時に呼び出されます。
    /// この関数を実装することで、各ステージに対して初期処理を行うことができます。
    /// @param[in] aStageAccessor 現在のステージ。
    void Answer::Init(const StageAccessor& aStageAccessor)
    {
        sTimer = 0;
        
    }

    /// 各ターンでの動作を返します。
    /// @param[in] aStageAccessor 現在ステージの情報。
    /// @return これから行う動作を表す Action クラス。
    Action Answer::GetNextAction(const StageAccessor& aStageAccessor)
    {
        ++sTimer; // turn_counter

        const Chara& player = aStageAccessor.player();
        const LotusCollection& lotuses = aStageAccessor.lotuses();
        const EnemyAccessor& enemies = aStageAccessor.enemies();
        
        bool isGuard = false;
        Vec2 nextVec = player.pos() + lotuses[player.targetLotusNo()].pos();

        
        for (int i = 0; i < enemies.count(); ++i)
        {
            Chara enemy = enemies.operator[](i);
            Circle enemyCircle = Circle(enemy.pos() + enemy.vel(), enemy.region().radius());
            if (Collision::IsHit(enemyCircle, player.region(), nextVec)) {
                isGuard = true;
                break;
            }
            
        }


        if (player.accelCount() > 0 && !isGuard){
            return Action::Accel(lotuses[player.targetLotusNo()].pos());
        }
        return Action::Wait();
    }
    
}

//------------------------------------------------------------------------------
// EOF
