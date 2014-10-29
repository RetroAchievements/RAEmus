using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Net.Http;
using System.Threading.Tasks;
using System.Collections;
using System.Runtime.Serialization.Json;
using System.Xml;
using System.Xml.Linq;
using System.ComponentModel;

namespace RAInterface
{
    public static class RAWebInterface
    {
        private static HttpClient client = new HttpClient();

        private static Queue<RAHttpRequest> HttpRequests = new Queue<RAHttpRequest>();

        //  Dirty args!
        public static void PerformBackgroundLogin(string user, string pass)
        {
            RACore.LocalUser = new RAUser( user );

            var args = new RAHttpRequest.PostArgs();

            args.Clear();
            args.Add(new KeyValuePair<string, string>("u", user));
            args.Add(new KeyValuePair<string, string>("p", pass));

            HttpRequests.Enqueue(new RAHttpRequest(WebRequest.RequestLogin, args));
        }

        public static void GetFriendList()
        {
            var args = new RAHttpRequest.PostArgs();

            args.Clear();
            args.Add(new KeyValuePair<string, string>("u", RACore.LocalUser.User));
            args.Add(new KeyValuePair<string, string>("t", RACore.LocalUser.Token));

            HttpRequests.Enqueue(new RAHttpRequest(WebRequest.RequestFriendList, args));
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
                case WebRequest.RequestLogin:
                    string User = root.Element("User").Value;
                    string Token = root.Element("Token").Value;
                    UInt64 UserScore = Convert.ToUInt64(root.Element("Score").Value);
                    UInt64 UserMessages = Convert.ToUInt64(root.Element("Messages").Value);

                    Console.WriteLine("{0} is logged in, {1} points, {2} messages", User, UserScore, UserMessages);

                    RACore.LocalUser.Setup(User, Token, UserScore, UserMessages);

                    RAWebInterface.GetFriendList();

                    RACore.CauseEvent(RAEventType.Login);

                    break;

                case WebRequest.RequestFriendList:
                    //  GetFriendList 
                    XElement friendList = root.Element("Friends");
                    if (friendList != null)
                    {
                        foreach (var friend in friendList.Elements())
                        {
                            string Friend = friend.Element("Friend").Value;
                            int FriendScore = Convert.ToInt32(friend.Element("RAPoints").Value);
                            string FriendActivity = friend.Element("LastSeen").Value;
                            //friend;
                            Console.WriteLine("{0} ({1}): {2}", Friend, FriendScore, FriendActivity);
                        }
                    }
                    break;

                case WebRequest.RequestScore:

                    break;

                case WebRequest.RequestPostActivity:
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

    }
}
