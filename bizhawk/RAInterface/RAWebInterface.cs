using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Net.Http;
using System.Threading.Tasks;
using System.Collections;

namespace RAInterface
{
    public static class RAWebInterface
    {
        public static string User { get; private set; }
        public static string UserToken { get; private set; }
        public static bool LoggedIn { get; private set; }

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

            values.Clear();
            values.Add(new KeyValuePair<string, string>("u", user));

            HttpRequests.Enqueue(new RAHttpRequest(WebRequestType.GetFriendList, values));
        }

        public static void PerformBlockingLogin(string user, string pass)
        {

        }

        public static void OnHttpResponse(Task<HttpResponseMessage> resp, RAHttpRequest req)
        {
           //  = ( Task < HttpResponseMessage >)(obj);
            MessageBox.Show("test", resp.ToString());
        }

        public static void DoRequest(RAHttpRequest req)
        {
            //  ##RA test code:
            {
                Task<HttpResponseMessage> PostMsg = client.PostAsync(req.TargetURL, req.Args);
                PostMsg.ContinueWith(_ => OnHttpResponse(_, req));
                //PostMsg.ContinueWith(_ => OnHttpResponse(_));
                //PostMsg.ContinueWith( requestTask => 
                //{
                //    var response = requestTask.Result;
                //});

                //Task < HttpResponseMessage > msg = client.PostAsync(req.TargetURL, req.Args);//.ContinueWith( t => OnHttpResponse( t ) )
                //await OnHttpResponse( msg );

                //Task<HttpResponseMessage> msg = client.PostAsync(req.TargetURL, req.Args);//.ContinueWith(OnHttpResponse);
                ////msg.GetAwaiter().OnCompleted();
                //msg.ContinueWith(OnHttpResponse);
                //msg.Start();
                //client.PostAsync(req.TargetURL, req.Args).ContinueWith(OnHttpResponse);

                //response.Wait();

                //var responseString = response.Result.Content.ReadAsStringAsync();
                //MessageBox.Show("Login result: " + responseString.Result);
                //if (responseString.Result.Substring(0, 3) == OKReply)
                //{
                //    UserToken = responseString.Result.Substring(OKReply.Length, TokenLength);
                //    LoggedIn = true;

                //    var values = new List<KeyValuePair<string, string>>();

                //    values.Clear();
                //    values.Add(new KeyValuePair<string, string>("u", User));
                //    values.Add(new KeyValuePair<string, string>("t", UserToken));
                //    values.Add(new KeyValuePair<string, string>("a", Convert.ToString((int)ActivityType.StartedPlaying)));
                //    values.Add(new KeyValuePair<string, string>("m", "30"));           //  game ID ( 1 = Sonic (Mega Drive), 30 = Jet Force Gemini (N64) )

                //    response = client.PostAsync(BaseURL + ActivityPage, new FormUrlEncodedContent(values));
                //    response.Wait();
                //    responseString = response.Result.Content.ReadAsStringAsync();
                //    MessageBox.Show("Post Activity result: " + responseString.Result);
                //}
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
