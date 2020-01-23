using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


using Windows.ApplicationModel.Email;
using LightBuzz.SMTP;
using Windows.Foundation.Collections;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EmailSupport
{
    [Windows.UI.Xaml.Data.Bindable]
    public class eMailing : INotifyPropertyChanged
    {
        IList<string> m_Receipients;
        string m_server;
        double m_port;
        bool m_ssl;
        string m_userName;
        string m_password;
        bool m_MailSended;
        public event PropertyChangedEventHandler PropertyChanged = null;

        public eMailing()
        {
            m_Receipients = new List<string>();
            m_server = "xyz.de";
            m_port = 465;
            m_ssl = true;
            m_userName ="John.Do.xyz";
            m_password = "xxxxx";
            m_MailSended = false;
            m_Receipients.Add("John.Do.xyz");
            m_Receipients.Add("");
            m_Receipients.Add("");
            m_Receipients.Add("");
        }


        public void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            if (this.PropertyChanged != null)
            {
                // Raise the PropertyChanged event, passing the name of the property whose value has changed.
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }


        }



        public bool MailSended
        {
            get { return m_MailSended; }
            set
            {
                m_MailSended = value;
                OnPropertyChanged();
            }

        }


        public string Server
        {
            get { return m_server; }
            set
            {
                m_server = value;
                OnPropertyChanged();
            }
                
        }
        public string UserName
        {
            get { return m_userName; }
            set
            {
                m_userName = value;
                OnPropertyChanged();
            }

        }
        public string Password
        {
            get { return m_password; }
            set
            {
                m_password = value;
                OnPropertyChanged();
            }

        }
        public double Port
        {
            get { return m_port; }
            set
            {
                m_port = value;
                OnPropertyChanged();
            }

        }
        public bool SSL
        {
            get { return m_ssl; }
            set
            {
                m_ssl = value;
                OnPropertyChanged();
            }

        }

        public IList<string> Receipients
        {
            get { return m_Receipients; }
            set
            {
                m_Receipients = value;
                OnPropertyChanged();
            }

        }
        public Windows.Foundation.IAsyncOperation<Boolean> SendMailAsync(string subject, string body)
        {
            Task<Boolean> from = sendInternalAsync(subject, body);

            Windows.Foundation.IAsyncOperation<Boolean> to = from.AsAsyncOperation();

            return to;


        }


        private async Task<bool> sendInternalAsync(string subject, string body)
        {
            MailSended = false;
//            using (SmtpClient client = new SmtpClient("smtp.example.com", 465, false, "SenderEmail@example.com", "YourPassword"))
            using (SmtpClient client = new SmtpClient(this.m_server,(int) this.m_port, this.m_ssl, this.m_userName, this.m_password))

            {
                EmailMessage emailMessage = new EmailMessage();

                for (int i = 0; i < m_Receipients.Count; i++)
                {
                    if (m_Receipients[i].Length > 0)
                    {
                        emailMessage.To.Add(new EmailRecipient(m_Receipients[i]));
                    }

                }
                emailMessage.Subject = subject;
                emailMessage.Body = body;

                var ret = await client.SendMailAsync(emailMessage);

                MailSended = (ret == SmtpResult.OK);
                return m_MailSended;

   
            }

        }
    }
}
