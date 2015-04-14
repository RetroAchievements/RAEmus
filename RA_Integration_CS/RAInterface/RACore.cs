using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RAInterface
{
    //  Console ID ordering should match up to DB
    public enum RAConsoleID
    {
        MegaDrive = 1,
        Nintendo64,
        SNES,
        Gameboy,
        GameboyAdvance,
        GameboyColor,
        NES,
        PCEngine,
        MasterSystem,
    };

    public enum RAEventType
    {
        Login,
        NewGameLoaded,
    };

    public enum RAActivityType
    {
        Unknown = 0,
        EarnedAchivement,
        Login,
        StartedPlaying,
        UploadAchievement,
        EditAchievement,
        CompleteGame,
        NewLeaderboardEntry,
        ImprovedLeaderboardEntry,

        NumActivityTypes
    };

    public enum RAObjectType
    {
        Game,
        User,
        Achievement,

        NumObjectTypes
    };

    public enum RAAchievementSet
    {
        Core,
        Unofficial,
        Local,

        NumAchievementSets
    };

    public static class RACore
    {
        public const int TokenLength = 16;
        public static RAUser LocalUser = new RAUser("");
        public static RAEventService EventService = new RAEventService();

        public static AchievementSet CoreAchievements = new AchievementSet();
        public static AchievementSet UnofficialAchievements = new AchievementSet();
        public static AchievementSet LocalAchievements = new AchievementSet();
        public static RAAchievementSet ActiveAchievementSet;

        public static void Init()
        {
            Console.WriteLine("RACore.Init, Ver" + RABuildVer.BuildVer);
        }

        public static void OnLoad(RAConsoleID consoleID, string gameTitle, string hash)
        {
            MessageBox.Show(gameTitle + " -> " + hash);
        }

        public static void Update()
        {
            RAWebInterface.Update();
            EventService.Update();

            if (ActiveAchievementSet == RAAchievementSet.Core)
                CoreAchievements.Update();
            else if (ActiveAchievementSet == RAAchievementSet.Unofficial)
                UnofficialAchievements.Update();
            else //(ActiveAchievementSet == RAAchievementSet.Local)
                LocalAchievements.Update();

            //if (Global.Emulator.MemoryDomains.Count() > 0)
            //{
            //    byte nTest = Global.Emulator.MemoryDomains.ElementAt(0).PeekByte(0x0010);
            //    // Console.WriteLine(nTest);
            //}
        }
    }
}
