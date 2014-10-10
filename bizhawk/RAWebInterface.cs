using System;
using System.Net.Http;

public static class RAWebInterface
{
    //  Dirty args!
    public static void PerformLogin( string user, string pass )
    {
        //  ##RA test code:

        using (var client = new HttpClient())
        {
            const string BaseURL = "http://retroachievements.org/";
            const string LoginPage = "requestlogin.php";
            const string ActivityPage = "requestpostactivity.php";

            string userName = "qwe";
            string pass = "qwe";    //  Yuck. Plaintext?!?

            var values = new List<KeyValuePair<string, string>>();

            values.Clear();
            values.Add(new KeyValuePair<string, string>("u", userName));
            values.Add(new KeyValuePair<string, string>("p", pass));

            System.Threading.Tasks.Task<HttpResponseMessage> response = client.PostAsync(BaseURL + LoginPage, new FormUrlEncodedContent(values));
            response.Wait();
            var responseString = response.Result.Content.ReadAsStringAsync();
            MessageBox.Show("Login result: " + responseString.Result);
            if (responseString.Result.Substring(0, 3) == "OK:")
            {
                string userToken = responseString.Result.Substring(3, 16);

                values.Clear();
                values.Add(new KeyValuePair<string, string>("u", userName));
                values.Add(new KeyValuePair<string, string>("t", userToken));
                values.Add(new KeyValuePair<string, string>("a", "3"));         //  activity type (3=playing)
                values.Add(new KeyValuePair<string, string>("m", "30"));        //  game ID ( 1 = Sonic (Mega Drive), 30 = Jet Force Gemini (N64) )

                response = client.PostAsync(BaseURL + ActivityPage, new FormUrlEncodedContent(values));
                response.Wait();
                responseString = response.Result.Content.ReadAsStringAsync();
                MessageBox.Show("Post Activity result: " + responseString.Result);
            }
        }
    }


}
