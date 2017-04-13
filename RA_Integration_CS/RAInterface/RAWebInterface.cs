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

                    RACore.EventService.CauseEvent(RAEventType.Login);

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

//public static class RAWebInterface
//{
//    //  Dirty args!
//    public static void PerformLogin(string user, string pass)
//    {
//        //  ##RA test code:

//        using (var client = new HttpClient())
//        {
//            const string BaseURL = "http://retroachievements.org/";
//            const string LoginPage = "requestlogin.php";
//            const string ActivityPage = "requestpostactivity.php";

//            string userName = "qwe";
//            string pass2 = "qwe";    //  Yuck. Plaintext?!?

//            var values = new List<KeyValuePair<string, string>>();

//            values.Clear();
//            values.Add(new KeyValuePair<string, string>("u", userName));
//            values.Add(new KeyValuePair<string, string>("p", pass2));

//            System.Threading.Tasks.Task<HttpResponseMessage> response = client.PostAsync(BaseURL + LoginPage, new FormUrlEncodedContent(values));
//            response.Wait();
//            var responseString = response.Result.Content.ReadAsStringAsync();
//            MessageBox.Show("Login result: " + responseString.Result);
//            if (responseString.Result.Substring(0, 3) == "OK:")
//            {
//                string userToken = responseString.Result.Substring(3, 16);

//                values.Clear();
//                values.Add(new KeyValuePair<string, string>("u", userName));
//                values.Add(new KeyValuePair<string, string>("t", userToken));
//                values.Add(new KeyValuePair<string, string>("a", "3"));         //  activity type (3=playing)
//                values.Add(new KeyValuePair<string, string>("m", "30"));        //  game ID ( 1 = Sonic (Mega Drive), 30 = Jet Force Gemini (N64) )

//                response = client.PostAsync(BaseURL + ActivityPage, new FormUrlEncodedContent(values));
//                response.Wait();
//                responseString = response.Result.Content.ReadAsStringAsync();
//                MessageBox.Show("Post Activity result: " + responseString.Result);
//            }
//        }
//    }
//}