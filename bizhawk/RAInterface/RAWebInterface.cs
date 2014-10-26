using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Net.Http;
using System.Threading.Tasks;
using System.Collections;
using System.Runtime.Serialization.Json;
using System.Xml;
using System.Xml.Linq;

namespace RAInterface
{
    public static class RAWebInterface
    {
        public static string User { get; private set; }
        public static string UserToken { get; private set; }
        public static bool LoggedIn { get; private set; }
        public static long UserScore { get; private set; }
        public static long UserMessages { get; private set; }

        private static HttpClient client = new HttpClient();

        private static Queue<RAHttpRequest> HttpRequests = new Queue<RAHttpRequest>();

        const string OKReply = "OK:";
        const int TokenLength = 16;

        public enum ActivityType
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

        public enum ObjectType
        {
            Game,
            User,
            Achievement,

            NumObjectTypes
        };


        //  Dirty args!
        public static void PerformBackgroundLogin(string user, string pass)
        {
            User = user;

            var values = new List<KeyValuePair<string, string>>();

            values.Clear();
            values.Add(new KeyValuePair<string, string>("u", user));
            values.Add(new KeyValuePair<string, string>("p", pass));

            HttpRequests.Enqueue(new RAHttpRequest(WebRequestType.Login, values));
        }

        public static void GetFriendList()
        {
            var values = new List<KeyValuePair<string, string>>();

            values.Clear();
            values.Add(new KeyValuePair<string, string>("u", User));
            values.Add(new KeyValuePair<string, string>("t", UserToken));

            HttpRequests.Enqueue(new RAHttpRequest(WebRequestType.GetFriendList, values));
        }

        public static void PerformBlockingLogin(string user, string pass)
        {

        }

        public async static void OnHttpResponse(Task<HttpResponseMessage> resp, RAHttpRequest req)
        {
            //string s = await resp.Result.Content.ReadAsStringAsync();
            byte[] b = await resp.Result.Content.ReadAsByteArrayAsync();
            XmlDictionaryReader r = JsonReaderWriterFactory.CreateJsonReader(b, new XmlDictionaryReaderQuotas());
            XElement root = XElement.Load(r);

            switch (req.RequestType)
            {
                case WebRequestType.Login:
                    User = root.Element("User").Value;
                    UserToken = root.Element("Token").Value;
                    UserScore = Convert.ToInt64( root.Element("Score").Value );
                    UserMessages = Convert.ToInt64( root.Element("Messages").Value );

                    Console.WriteLine("{0} is logged in, {1} points, {2} messages", User, UserScore, UserMessages);

                    RAWebInterface.GetFriendList();

                    break;
                case WebRequestType.GetFriendList:
                    //  GetFriendList 
                    XElement friendList = root.Element("Friends");
                    foreach (var friend in friendList.Elements())
                    {
                        string Friend = friend.Element("Friend").Value;
                        int FriendScore = Convert.ToInt32(friend.Element("RAPoints").Value);
                        string FriendActivity = friend.Element("LastSeen").Value;
                        //friend;
                        Console.WriteLine("{0} ({1}): {2}", Friend, FriendScore, FriendActivity);
                    }
                    break;
                case WebRequestType.PostActivity:
                    //  FAF
                    break;
            }
        }

        public static void DoRequest(RAHttpRequest req)
        {
            //  ##RA test code:
            Task<HttpResponseMessage> PostMsg = client.PostAsync(req.TargetURL, req.Args);
            PostMsg.ContinueWith(_ => OnHttpResponse(_, req));
        }

        public static bool RequestsExist
        {
            get
            {
                return (HttpRequests.Count > 0);
            }
        }

        public static void Update()
        {
            if (HttpRequests.Count > 0)
            {
                RAHttpRequest req = HttpRequests.Dequeue();
                DoRequest(req);
            }
        }

        public static bool IsLoggedIn
        {
            get
            {
                return (UserToken != null);
            }
        }
    }
}
