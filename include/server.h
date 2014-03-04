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

#ifndef SHAPLIM_SERVER_H
#define SHAPLIM_SERVER_H

#include <memory>
#include <string>
#include <functional>
#include <array>
#include <boost/asio.hpp>
#include <jsoncpp/json/value.h>

class session : public std::enable_shared_from_this<session> {
public:
	using socket_type = boost::asio::ip::tcp::socket;
	using callback_type = std::function<Json::Value(session&, std::string)>;

	session(socket_type sock, callback_type callback);
	void start();
	void close();
private:
	using buffer_type = boost::asio::streambuf;

	void do_read();
	void do_write();

	socket_type m_socket;
	buffer_type m_read_buffer;
	std::string m_send_buffer;
	callback_type m_callback;
};

class server {
public:
	using socket_type = boost::asio::ip::tcp::socket;
	using callback_type = session::callback_type;

	server(boost::asio::io_service& io_service, short port);

	void on_data_available(callback_type callback);
private:
	void do_accept();

	boost::asio::ip::tcp::acceptor m_acceptor;
	socket_type m_socket;
	callback_type m_callback;
};

#endif // SHAPLIM_SERVER_H
