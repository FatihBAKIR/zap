import * as zap from "./zap"
import express = require("express");

const tok = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0eXBlIjowLCJvYmoiOjEsInBlcm0iOjAsImlhdCI6MTU1MDM4NzE1MywiZXhwIjoxNTUwNDczNTUzfQ.VRPrEwnTtpwshaw-lif-kUpGVmMicgFqeW8DjTTq9QE";


const app = express();
const port = 8081;

app.get("/req/:id/:ip", async(req, res) => {
    try{
        const buf = zap.make_request("handle_ip", tok, zap.make_ip(parseInt(req.params.id), req.params.ip));

        res.json(await zap.do_req(buf));
    }
    catch (e)
    {
        res.status(400);
        res.json(e);
    }
});

app.listen(port, async() => console.log(`App listening on port ${port}!`));
