/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <iostream>
#include <string>
#include <chrono>
#include <jsoncpp/json/writer.h>
#include "server.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

session::session(socket_type sock, callback_type callback)
: m_socket(std::move(sock)), m_read_buffer(), m_callback(std::move(callback))
{

}

void session::start() 
{
	do_read();
}

void session::do_read()
{
	auto self = shared_from_this();
    boost::asio::async_read_until(
    	m_socket,
    	m_read_buffer,
    	'\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
         	if (!ec) {
         		std::istream is(&m_read_buffer);
         		std::string str;
         		std::getline(is, str);
         		try {
	        		auto result = m_callback(*this, std::move(str));
	        		Json::FastWriter writer;
	        		m_send_buffer = writer.write(result);
	        		do_write();
	        	}
	        	catch(std::exception& ex) { 
	        		close();
	        	}
          	}
          	else
          		close();
        }
    );
}

void session::close() 
{
	boost::system::error_code ignored_ec;
    m_socket.shutdown(
    	tcp::socket::shutdown_both,
        ignored_ec
    );
	m_socket.close();
}

void session::do_write()
{
	auto self = shared_from_this();
	boost::asio::async_write(
		m_socket, 
		boost::asio::buffer(m_send_buffer), 
		[this, self](boost::system::error_code ec, std::size_t) { 
			if(!ec)
				do_read();
			else
				close();
		}
	);
}

// server

server::server(boost::asio::io_service& io_service, short port)
: m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
m_socket(io_service)
{
	do_accept();
}

void server::do_accept()
{
	m_acceptor.async_accept(
		m_socket,
   	 	[this](boost::system::error_code ec) {
   	 		if(!m_callback)
   	 			throw std::runtime_error("No callback has been set");
      		if (!ec)
        		std::make_shared<session>(std::move(m_socket), m_callback)->start();
			do_accept();
    	}
    );
}

void server::on_data_available(callback_type callback)
{
	m_callback = std::move(callback);
}

// service_discovery_server

service_discovery_server::service_discovery_server(boost::asio::io_service& io_service, short port)
: m_socket(io_service, udp::endpoint(udp::v4(), port))
{
	start_receive();
}

void service_discovery_server::set_data_to_answer(std::string data) 
{
	m_data_to_answer = std::move(data);
}

void service_discovery_server::start_receive() 
{
	m_socket.async_receive_from(
    	boost::asio::buffer(m_read_buffer), 
    	m_remote_endpoint,
        std::bind(
        	&service_discovery_server::handle_receive, 
        	this,
          	std::placeholders::_1,
          	std::placeholders::_2
        )
	);
}

void service_discovery_server::handle_receive(
	const boost::system::error_code& error, std::size_t) 
{
	if(!error || error == boost::asio::error::message_size) {
      	m_socket.async_send_to(
          boost::asio::buffer(m_data_to_answer), 
            m_remote_endpoint,
            std::bind(
              &service_discovery_server::handle_send, 
              this, 
              std::placeholders::_1,
              std::placeholders::_2
            )
        );

      	start_receive();
    }
}

void service_discovery_server::handle_send(
	const boost::system::error_code&, std::size_t)
{

}
