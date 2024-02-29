const express = require("express")
const axios = require('axios')
const cors = require("cors");

const app = express()
app.use(cors())

CLIENT_ID = "spotify-client-id"
CLIENT_SECRET = "spotify-secret"
PORT = 5173 // it is located in Spotify dashboard's Redirect URIs, my port is 3000
REDIRECT_URI = `http://192.168.50.112/callback/` // my case is 'http://localhost:3000/callback'
SCOPE = [
    "user-read-playback-state",
    "user-read-currently-playing"
]

app.get("/login", (request, response) => {
    const redirect_url = `https://accounts.spotify.com/authorize?response_type=code&client_id=${CLIENT_ID}&scope=${SCOPE}&state=123456&redirect_uri=${REDIRECT_URI}&prompt=consent`
    response.redirect(redirect_url);
})

app.get("/callback", async (request, response) => {
    const code = request.query["code"]
    await axios.post(
        url = 'https://accounts.spotify.com/api/token',
        data = new URLSearchParams({
            'grant_type': 'authorization_code',
            'redirect_uri': REDIRECT_URI,
            'code': code
        }),
        config = {
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            params: {
                'grant_type': 'authorization_code'
            },
            auth: {
                username: CLIENT_ID,
                password: CLIENT_SECRET
            }
        })
        .then(resp1 => {
            return response.send(JSON.stringify(resp1.data));
        });

})

app.listen(PORT, () => {
    console.log(`Listening on :${PORT}`)
})
