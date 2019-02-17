import { bb } from "./ip_addr_generated";
import { bb as req} from "./req_generated";
import {flatbuffers} from "flatbuffers";
import * as dgram from "dgram";

export function make_ip(gw_id: number, addr: string) : Uint8Array
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

export function make_request(handler: string, token: string, body: Uint8Array) : Uint8Array
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

export function do_req(buf : Uint8Array) : Promise<any>
{
    return new Promise<any>((res, rej) => {
        const client = dgram.createSocket('udp4');

        const tmr = setTimeout(() => {
            rej("timeout");
            client.close();
        }, 2000);
    
        client.send(buf, 9993, "localhost", (err) => {
            if (err)
            {
                clearTimeout(tmr);
                client.close();
                rej(err);
                return;
            }
        });
    
        client.on("message", (msg, rinfo) => {
            res(JSON.parse(msg.toString("utf8")));
            clearTimeout(tmr);
            client.close();
        });
    });
}