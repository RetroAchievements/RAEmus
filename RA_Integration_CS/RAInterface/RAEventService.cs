using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RAInterface
{
    public class RAEventService
    {
        public class RAEventArgs : EventArgs
        {
            public object Data { get; private set; }

            public RAEventArgs(object data)
                : base()
            {
                Data = data;
            }
        }

        private EventHandlerList _SubscribedEvents = new EventHandlerList();
        private Queue<Tuple<RAEventType, RAEventArgs>> _EventsToTriggerOnMainThread = new Queue<Tuple<RAEventType, RAEventArgs>>();

        public void RegisterHandler(RAEventType type, EventHandler handler)
        {
            _SubscribedEvents.AddHandler(type.ToString(), handler);
        }

        public void UnregisterHandler(RAEventType type, EventHandler handler)
        {
            _SubscribedEvents.RemoveHandler(type.ToString(), handler);
        }

        public void CauseEvent(RAEventType type, RAEventArgs e = null)
        {
            _EventsToTriggerOnMainThread.Enqueue(new Tuple<RAEventType, RAEventArgs>(type, e));
        }

        private void PublishEvent(EventHandler handler, RAEventArgs e)
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

        public void Update()
        {
            while (_EventsToTriggerOnMainThread.Count() > 0)
            {
                Tuple<RAEventType, RAEventArgs> toTrigger = _EventsToTriggerOnMainThread.Dequeue();
                var handler = _SubscribedEvents[toTrigger.Item1.ToString()] as EventHandler;
                PublishEvent(handler, toTrigger.Item2);
            }
        }

    }
}
