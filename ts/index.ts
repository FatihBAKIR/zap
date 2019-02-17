import { bb } from "./ip_addr_generated";
import { bb as req} from "./req_generated";
import {flatbuffers} from "flatbuffers";
import * as dgram from "dgram";

function make_ip(gw_id: number, addr: string) : Uint8Array
{
    const builder = new flatbuffers.Builder(64);
    const ip = builder.createString(addr);
    
    bb.cloud.IPAddr.startIPAddr(builder);
    bb.cloud.IPAddr.addAddr(builder, ip);
    bb.cloud.IPAddr.addGwId(builder, gw_id);
    const addr_off = bb.cloud.IPAddr.endIPAddr(builder);
    builder.finish(addr_off);
    
    return builder.asUint8Array();
}

function make_request(handler: string, token: string, body: Uint8Array) : Uint8Array
{
    const builder = new flatbuffers.Builder(128);
    const handler_off = builder.createString(handler);
    const token_off = builder.createString(token);
    const body_off = req.cloud.Request.createBodyVector(builder, body);

    req.cloud.Request.startRequest(builder);
    req.cloud.Request.addHandler(builder, handler_off);
    req.cloud.Request.addToken(builder, token_off);
    req.cloud.Request.addBody(builder, body_off);
    const addr = req.cloud.Request.endRequest(builder);
    builder.finish(addr);

    return builder.asUint8Array();
}

const tok = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0eXBlIjowLCJvYmoiOjEsInBlcm0iOjAsImlhdCI6MTU1MDM4NzE1MywiZXhwIjoxNTUwNDczNTUzfQ.VRPrEwnTtpwshaw-lif-kUpGVmMicgFqeW8DjTTq9QE";

const buf = make_request("handle_ip", tok, make_ip(2, "192.168.2.20"));

console.log(buf.length);

const client = dgram.createSocket('udp4');
client.send(buf, 9993, "localhost", (err, bytes) => {
    client.close();
    if (err)
    {
        console.log(err);
        return;
    }
    console.log("success");
});
