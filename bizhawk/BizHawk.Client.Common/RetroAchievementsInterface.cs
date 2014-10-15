using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using RAInterface;

namespace BizHawk.Client.Common
{
    public class RetroAchievementsInterface
    {
        //  Console ID ordering should match up to DB
        public enum ConsoleID
        {
            MegaDrive = 1,
            Nintendo64,
            SNES,
            Gameboy,
            GameboyAdvance,
            GameboyColor,
            NES,
            PCEngine,
        };

        public AchievementSet LocalAchievements;
        public AchievementSet UnofficialAchievements;
        public AchievementSet CoreAchievements;

        public void OnLoad(ConsoleID id, RomGame rom)
        {
            //rom.

            //Global.CheatList.Add(
        }
    }
}
