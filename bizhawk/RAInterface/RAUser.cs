using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RAInterface
{
    public class RAUser
    {
        public RAUser(string user)
        {
            _User = user;
            Token = null;
            UserScore = 0;
            UserMessages = 0;
        }

        public readonly string _User;

        public string User
        {
            get
            {
                return _User;
            }
        }
        public string Token { get; private set; }
        public UInt64 UserScore { get; private set; }
        public UInt64 UserMessages { get; private set; }

        public bool IsLoggedIn
        {
            get
            {
                return (Token != null);
            }
        }

        public void Setup(string user, string token, UInt64 userScore, UInt64 userMessages)
        {
            //User = user;
            Token = token;
            UserScore = userScore;
            UserMessages = userMessages;
            //  Persist at this point?
        }

        public string TitleMessage()
        {
            if (IsLoggedIn)
                return string.Format("( Logged in: {0}({1}), {2} unread )", User, UserScore, UserMessages);
            else
                return "";
        }
    }
}
