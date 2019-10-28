package classes;

import android.os.AsyncTask;
import android.util.Log;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.UnknownHostException;


//executes an http get request as a background asynchronous task
public class HttpGetRequest extends AsyncTask<String, Void, String> {
    private static final String REQUEST_METHOD = "GET";
    private static final int READ_TIMEOUT = 15000;
    private static final int CONNECTION_TIMEOUT = 15000;

    @Override
    protected String doInBackground(String... params) {
        String urlString = params[0];
        Log.d("INIT", "value of passed in address in get request class: " + urlString);
        String result;
        String inputLine;

        try{
            //Create URL object to hold passed in URL
            URL myUrl = new URL(urlString);
            HttpURLConnection connection;

            //Create the connection
            try{
                connection = (HttpURLConnection) myUrl.openConnection();
            }
            catch(UnknownHostException e){
                Log.e("SPECIAL", "unknown host");
                return null;
            }

            //Set methods and timeouts
            connection.setRequestMethod(REQUEST_METHOD);
            connection.setReadTimeout(READ_TIMEOUT);
            connection.setConnectTimeout(CONNECTION_TIMEOUT);

            //Connect to the url
            connection.connect();

            //Creating an Input Stream Reader
            InputStreamReader streamReader = new InputStreamReader(connection.getInputStream());

            //Creating new buffered reader and String Builder
            BufferedReader reader = new BufferedReader(streamReader);
            StringBuilder stringBuilder = new StringBuilder();

            //read in the response one line at a time
            while((inputLine = reader.readLine()) != null){
                stringBuilder.append(inputLine);
            }

            //Close input stream and buffered reader
            reader.close();
            streamReader.close();

            //Turn stringbuilder result into return string
            result = stringBuilder.toString();

        }
        catch(IOException e){
            e.printStackTrace();
            result = "";
        }
        return result;
    }

    @Override
    protected void onPostExecute(String result){
        super.onPostExecute(result);
    }
}
