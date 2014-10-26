using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Security.Cryptography;
using RAInterface;
using System.Windows.Forms;

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
            //MD5 hasher = System.Security.Cryptography.MD5.Create();
            //byte[] hash = hasher.ComputeHash(rom.RomData);
            
            //StringBuilder sb = new StringBuilder();
            //for (int i = 0; i < hash.Length; i++)
            //    sb.Append(hash[i].ToString("X2"));

            MessageBox.Show(rom.GameInfo.Name + " -> " + rom.GameInfo.Hash);

        }

        public void Update()
        {
            RAWebInterface.Update();

            if (Global.Emulator.MemoryDomains.Count() > 0)
            {
                byte nTest = Global.Emulator.MemoryDomains.ElementAt(0).PeekByte(0x0010);
                Console.WriteLine(nTest);
            }
        }
    }
}
