// ----------------------------------------------------
    // 上の関数群（SyncServoCenterなど）はそのまま変更なしでOK
    // ----------------------------------------------------

    void setup()
    {
        HAL_Delay(100); // 起動直後の長めの安定待ち
        
        // 1. 初期位置を取得 (1回だけ実行)
        startPos = SyncServoCenter(servoID);
        
        HAL_Delay(1000);
    }
    
    void loop()
    {
        // 2. 例：初期位置から180度の位置へ移動する量を計算
        int16_t offset = deg_to_ticks(180.0f);
        
        // 移動命令を送る (main.cのwhileで何度も呼ばれる)
        MoveServoRelative(servoID, startPos, offset);
        
        HAL_Delay(5000); // 5秒待つ
    }
} // namespace run の閉じカッコ

extern "C" void setup_c()
{
    run::setup();
}

extern "C" void loop_c()
{
    run::loop();
}
