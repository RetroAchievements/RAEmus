using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RAInterface
{
    public enum RAEventType
    {
        Login,
        NewGameLoaded,
    };

    public static class RACore
    {
        public const int TokenLength = 16;
        public static RAUser LocalUser = new RAUser("");

        public class RAEventArgs : EventArgs
        {
            public object Data { get; private set; }

            public RAEventArgs(object data)
                : base()
            {
                Data = data;
            }
        }

        private static EventHandlerList _SubscribedEvents = new EventHandlerList();
        private static Queue<Tuple<RAEventType, RAEventArgs>> _EventsToTriggerOnMainThread = new Queue<Tuple<RAEventType, RAEventArgs>>();

        public static void RegisterHandler(RAEventType type, EventHandler handler)
        {
            _SubscribedEvents.AddHandler(type.ToString(), handler);
        }

        public static void UnregisterHandler(RAEventType type, EventHandler handler)
        {
            _SubscribedEvents.RemoveHandler(type.ToString(), handler);
        }

        public static void CauseEvent(RAEventType type, RAEventArgs e = null)
        {
            _EventsToTriggerOnMainThread.Enqueue(new Tuple<RAEventType, RAEventArgs>(type, e));
            //var handler = _SubscribedEvents[type.ToString()] as EventHandler;
            //PublishEvent(handler, e);
        }

        private static void PublishEvent(EventHandler handler, RAEventArgs e)
        {
            try
            {
                if (handler != null)
                    handler.Invoke(null, e);
            }
            catch (Exception /*ex*/)
            {
                //Debug.Fail("Cannot call event: " + ex.ToString());
            }
        }

        public static void UpdateEventService()
        {
            while (_EventsToTriggerOnMainThread.Count() > 0)
            {
                Tuple<RAEventType, RAEventArgs> toTrigger = _EventsToTriggerOnMainThread.Dequeue();
                var handler = _SubscribedEvents[toTrigger.Item1.ToString()] as EventHandler;
                PublishEvent(handler, toTrigger.Item2);
            }
        }


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


    }
}
